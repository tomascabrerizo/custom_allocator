@echo off

set TARGET=mem.exe
set CC=clang++
set CFLAGS=-O0 -g -Wall -Wextra -Werror

if not exist .\build mkdir .\build

%CC% %CFLAGS% memory.cpp arena.cpp heap.cpp mem_test.cpp -o ./build/%TARGET%
