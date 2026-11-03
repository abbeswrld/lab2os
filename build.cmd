@echo off
setlocal
cd /d "%~dp0"
if not exist "build" mkdir "build"

gcc -std=c11 -Wall -Wextra -Wpedantic -O2 -c bg_win.c -o build\bg.o
if errorlevel 1 exit /b 1
ar rcs build\libbg.a build\bg.o
if errorlevel 1 exit /b 1
gcc -std=c11 -Wall -Wextra -Wpedantic -O2 main.c -Lbuild -lbg -o build\lab2.exe
if errorlevel 1 exit /b 1

echo Built: build\lab2.exe
echo Run:   build\lab2.exe
exit /b 0
