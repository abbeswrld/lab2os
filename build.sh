#!/usr/bin/env sh
set -eu
cd "$(dirname "$0")"
mkdir -p build
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 -c bg_posix.c -o build/bg.o
ar rcs build/libbg.a build/bg.o
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 main.c -Lbuild -lbg -o build/lab2
printf 'Built: build/lab2\nRun:   ./build/lab2\n'
