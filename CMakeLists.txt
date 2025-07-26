cmake_minimum_required(VERSION 3.10)

set(C_STANDARD 11)
set(C_STANDARD_REQUIRED ON)

project(API)

enable_testing()

add_executable(
    movehex
    src/main.c 
)

set(CMAKE_C_COMPILE_FEATURES "-O2 -DEVAL -std=gnu11 -Wall -Werror -pipe -static -s -lm -g3")

add_subdirectory(test)

