cmake_minimum_required(VERSION 3.10)

project(API_progetto)

add_executable(
    movhex
    src/main.c 
)

set(BUILD_FLAGS -O2 -DEVAL -std=gnu11 -Wall -Werror -pipe -static -s -lm -g3)

target_compile_options(
    movhex PRIVATE ${BUILD_FLAGS}
)
