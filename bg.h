#ifndef BG_H
#define BG_H

#ifdef _WIN32
#include <windows.h>
typedef struct {
    HANDLE handle;
} bg_process_t;
#else
#include <sys/types.h>
typedef struct {
    pid_t pid;
} bg_process_t;
#endif

int bg_start(bg_process_t *process, const char *program,
             const char *const args[]);

int bg_wait(bg_process_t *process, int *exit_code);

#endif
