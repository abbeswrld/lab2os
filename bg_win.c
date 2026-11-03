#include "bg.h"

#include <stdlib.h>
#include <string.h>

static char *append_quoted(char *dst, const char *src) {
    *dst++ = '"';
    while (*src) {
        size_t slashes = 0;
        while (*src == '\\') {
            ++slashes;
            ++src;
        }
        if (*src == '"') {
            for (size_t i = 0; i < 2 * slashes + 1; ++i) {
                *dst++ = '\\';
            }
            *dst++ = '"';
            ++src;
        } else if (*src == '\0') {
            for (size_t i = 0; i < 2 * slashes; ++i) {
                *dst++ = '\\';
            }
        } else {
            for (size_t i = 0; i < slashes; ++i) {
                *dst++ = '\\';
            }
            *dst++ = *src++;
        }
    }
    *dst++ = '"';
    return dst;
}

int bg_start(bg_process_t *process, const char *program,
             const char *const args[]) {
    size_t total;
    char *cmdline;
    char *out;
    STARTUPINFOA startup = {0};
    PROCESS_INFORMATION info = {0};

    if (!process || !program || !*program || !args) {
        return ERROR_INVALID_PARAMETER;
    }
    process->handle = NULL;
    
    if (strlen(program) > (32767u - 4u) / 2u) {
        return ERROR_BAD_LENGTH;
    }
    total = 2 * strlen(program) + 4;
    for (size_t i = 0; args[i]; ++i) {
        size_t len = strlen(args[i]);
        if (total > 32767u - 4u || len > (32767u - total - 4u) / 2u) {
            return ERROR_BAD_LENGTH;
        }
        total += 2 * len + 4;
    }
    cmdline = malloc(total);
    if (!cmdline) {
        return ERROR_OUTOFMEMORY;
    }
    out = append_quoted(cmdline, program);
    for (size_t i = 0; args[i]; ++i) {
        *out++ = ' ';
        out = append_quoted(out, args[i]);
    }
    *out = '\0';

    startup.cb = sizeof(startup);
    if (!CreateProcessA(NULL, cmdline, NULL, NULL, FALSE, 0,
                        NULL, NULL, &startup, &info)) {
        int result = (int)GetLastError();
        free(cmdline);
        return result;
    }
    free(cmdline);
    CloseHandle(info.hThread);
    process->handle = info.hProcess;
    return 0;
}

int bg_wait(bg_process_t *process, int *exit_code) {
    DWORD status;
    DWORD code;

    if (!process || !process->handle || !exit_code) {
        return ERROR_INVALID_PARAMETER;
    }
    status = WaitForSingleObject(process->handle, INFINITE);
    if (status != WAIT_OBJECT_0) {
        return status == WAIT_FAILED ? (int)GetLastError() : ERROR_GEN_FAILURE;
    }
    if (!GetExitCodeProcess(process->handle, &code)) {
        return (int)GetLastError();
    }
    CloseHandle(process->handle);
    process->handle = NULL;
    *exit_code = (int)code;
    return 0;
}
