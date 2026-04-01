cmake_minimum_required(VERSION 3.10)

set(C_STANDARD 11)
set(C_STANDARD_REQUIRED ON)

project(API)

enable_testing()
add_subdirectory(test)

add_executable(
    movhex
    src/main.c 
)

# set(CMAKE_C_FLAGS "-O2 -DEVAL -std=gnu11 -Wall -Werror -pipe -static -s -g3")
target_link_options(movhex PRIVATE -static)

target_compile_features(movhex PRIVATE c_std_11)
target_compile_options(movhex PRIVATE 
        "$<IF:$<CXX_COMPILER_ID:MSVC>,"
        "/W4;$<$<CONFIG:RELEASE>:/O2>,"
        "-Wall;-Wextra;-Werror;-pipe;-DEVAL"
            "$<$<CONFIG:RELEASE>:-O3>"
        ">"
        $<$<CONFIG:Debug>:-g3>
)
