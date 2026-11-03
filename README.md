# Laboratory work 2: asynchronous processes (Windows / POSIX)

The task is to write a cross-platform C library that starts a child process in the background, allows the parent to continue, waits for the child on demand, and obtains its exit code.

## Files

- `bg.h` — the shared library interface, `bg_start` and `bg_wait`.
- `bg_win.c` — Windows implementation (`CreateProcessA`, `WaitForSingleObject`, `GetExitCodeProcess`).
- `bg_posix.c` — Linux/POSIX implementation (`fork`, `execvp`, `waitpid`). A close-on-exec pipe reports launch errors to the parent.
- `main.c` — test program. Starts its own executable as a child and checks exit code `7`; also checks the error when launching a missing program.
- `build.cmd`, `build.sh`, `Makefile` — build methods.

`libbg.a` is a static library created during the build. The tests are linked to it; it is not just a source file compiled into the executable.

## Windows (MinGW-w64 / MSYS2 UCRT64)

In the `lab2` directory, run:

```bat
build.cmd
build\lab2.exe
```

Requires `gcc` and `ar` available on the PATH. In MSYS2 UCRT64, both are included in the MinGW-w64 GCC toolchain.

## Linux (Ubuntu)

In the `lab2` directory, run:

```sh
sh build.sh
./build/lab2
```

Or use `make` and `make run`.

## Expected result

The parent starts the child *without waiting*. It prints a message that it can continue working and only then calls `bg_wait`. The child finishes with code `7`. The library returns that code to the parent. Launching a nonexistent program also returns an error. At the end, the program prints `All tests passed.` and exits with code `0`.

## Explanation for the instructor

`bg_start` does not wait for the child process and therefore demonstrates asynchronous execution. On POSIX, `fork()` creates a new process and `execvp()` replaces its code with the requested program. On Windows, `CreateProcessA()` starts the process and returns its process handle. `bg_wait` blocks until the child terminates using `waitpid()` (POSIX) or `WaitForSingleObject()` (Windows), then extracts the exit code. The process resources are released after a successful wait.

The standard output is inherited from the parent, not captured by the library. The returned "code" is the **process exit status**, not the child's printed text.
