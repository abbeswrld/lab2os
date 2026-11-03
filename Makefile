CC = gcc
AR = ar
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2

ifeq ($(OS),Windows_NT)
    PLATFORM_SRC = bg_win.c
    EXE = build/lab2.exe
else
    PLATFORM_SRC = bg_posix.c
    EXE = build/lab2
endif

.PHONY: all run clean
all: $(EXE)

build:
	mkdir -p build

build/bg.o: $(PLATFORM_SRC) bg.h | build
	$(CC) $(CFLAGS) -c $(PLATFORM_SRC) -o $@

build/libbg.a: build/bg.o
	$(AR) rcs $@ $<

$(EXE): main.c bg.h build/libbg.a
	$(CC) $(CFLAGS) main.c -Lbuild -lbg -o $@

run: all
	./$(EXE)

clean:
	rm -rf build
