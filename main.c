#include "bg.h"

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
static void child_delay(void) { Sleep(750); }
#else
#include <unistd.h>
static void child_delay(void) { sleep(1); }
#endif

int main(int argc, char *argv[]) {
    bg_process_t process = {0};
    const char *child_args[] = {"--child", "7", "argument with spaces", NULL};
    const char *bad_args[] = {NULL};
    int code;
    int error;
    
    if (argc > 1 && strcmp(argv[1], "--child") == 0) {
        if (argc != 4 || strcmp(argv[2], "7") != 0 ||
            strcmp(argv[3], "argument with spaces") != 0) {
            fprintf(stderr, "Child: incorrect arguments\n");
            return 3;
        }
        printf("Child: started; argument = [%s]\n", argv[3]);
        fflush(stdout);
        child_delay();
        puts("Child: finished; exit code = 7");
        return 7;
    }

    puts("Parent: launching child asynchronously...");
    fflush(stdout);
    error = bg_start(&process, argv[0], child_args);
    if (error != 0) {
        fprintf(stderr, "bg_start failed (OS error %d)\n", error);
        return 1;
    }

    puts("Parent: child is running; parent can do other work.");
    puts("Parent: now waiting for child to finish...");
    fflush(stdout);

    error = bg_wait(&process, &code);
    if (error != 0) {
        fprintf(stderr, "bg_wait failed (OS error %d)\n", error);
        return 1;
    }
    printf("Parent: child's exit code = %d\n", code);
    if (code != 7) {
        fprintf(stderr, "Test failed: expected exit code 7\n");
        return 1;
    }
    
#ifdef _WIN32
    error = bg_start(&process, "__no_such_program_lab2__.exe", bad_args);
#else
    error = bg_start(&process, "__no_such_program_lab2__", bad_args);
#endif
    if (error == 0) {
        fprintf(stderr, "Test failed: nonexistent program was launched\n");
        bg_wait(&process, &code);
        return 1;
    }
    printf("Parent: missing program correctly reported error %d\n", error);
    puts("All tests passed.");
    return 0;
}
