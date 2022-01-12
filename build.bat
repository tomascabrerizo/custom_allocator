@echo off

set TARGET=mem.exe
set CC=clang++
set CFLAGS=-O0 -g -Wall -Wextra -Werror

%CC% %CFLAGS% main.cpp -o %TARGET%
