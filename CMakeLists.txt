cmake_minimum_required(VERSION 3.10)

set(C_STANDARD 11)
set(C_STANDARD_REQUIRED ON)

project(API)

enable_testing()

add_executable(
    movehex
    src/main.c 
)

# set(CMAKE_C_FLAGS "-O2 -DEVAL -std=gnu11 -Wall -Werror -pipe -static -s -g3")
target_compile_options(movehex PRIVATE 
        -DEVAL -std=gnu11 -Wall -Werror -pipe -static
        $<$<CONFIG:Debug>:-g3 >
        $<$<CONFIG:Release>:-O2>
)

target_link_options(movehex PRIVATE -lm)

add_subdirectory(test)