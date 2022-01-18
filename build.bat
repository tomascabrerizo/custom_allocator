@echo off

set TARGET=mem.exe
set CC=clang++
set CFLAGS=-O0 -g -Wall -Wextra -Werror -Wno-unused-variable

if not exist .\build mkdir .\build

%CC% %CFLAGS% profiler.cpp memory.cpp arena.cpp heap.cpp mem_test.cpp -o ./build/%TARGET%
