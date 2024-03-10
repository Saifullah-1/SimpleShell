cmake_minimum_required(VERSION 3.27)
project(SimpleShell C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(SimpleShell
        main.c)
