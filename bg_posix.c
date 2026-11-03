#include "bg.h"

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int bg_start(bg_process_t *process, const char *program,
             const char *const args[]) {
    size_t count = 0;
    char **exec_args;
    int err_pipe[2];
    int flags;
    pid_t pid;
    int exec_error = 0;
    ssize_t n;

    if (!process || !program || !*program || !args) {
        return EINVAL;
    }
    process->pid = -1;

    while (args[count]) {
        if (count >= (SIZE_MAX / sizeof(char *)) - 2) {
            return E2BIG;
        }
        count++;
    }
    exec_args = malloc((count + 2) * sizeof(*exec_args));
    if (!exec_args) {
        return ENOMEM;
    }
    exec_args[0] = (char *)program;
    for (size_t i = 0; i < count; ++i) {
        exec_args[i + 1] = (char *)args[i];
    }
    exec_args[count + 1] = NULL;
    
    if (pipe(err_pipe) == -1) {
        int result = errno;
        free(exec_args);
        return result;
    }
    flags = fcntl(err_pipe[1], F_GETFD);
    if (flags == -1 || fcntl(err_pipe[1], F_SETFD, flags | FD_CLOEXEC) == -1) {
        int result = errno;
        close(err_pipe[0]);
        close(err_pipe[1]);
        free(exec_args);
        return result;
    }

    pid = fork();
    if (pid == -1) {
        int result = errno;
        close(err_pipe[0]);
        close(err_pipe[1]);
        free(exec_args);
        return result;
    }

    if (pid == 0) {
        int result;
        close(err_pipe[0]);
        execvp(program, exec_args);
        result = errno;
        
        while (write(err_pipe[1], &result, sizeof(result)) == -1 && errno == EINTR) {
        }
        _exit(127);
    }

    free(exec_args);
    close(err_pipe[1]);
    do {
        n = read(err_pipe[0], &exec_error, sizeof(exec_error));
    } while (n == -1 && errno == EINTR);
    {
        int read_error = (n == -1) ? errno : EIO;
        close(err_pipe[0]);
        if (n != 0) {
            
            while (waitpid(pid, NULL, 0) == -1 && errno == EINTR) {
            }
            return n == (ssize_t)sizeof(exec_error) ? exec_error : read_error;
        }
    }

    process->pid = pid;
    return 0;
}

int bg_wait(bg_process_t *process, int *exit_code) {
    int status;
    pid_t result;

    if (!process || process->pid <= 0 || !exit_code) {
        return EINVAL;
    }
    do {
        result = waitpid(process->pid, &status, 0);
    } while (result == -1 && errno == EINTR);
    if (result == -1) {
        return errno;
    }
    process->pid = -1;

    if (WIFEXITED(status)) {
        *exit_code = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        *exit_code = 128 + WTERMSIG(status);
    } else {
        return ECHILD;
    }
    return 0;
}
