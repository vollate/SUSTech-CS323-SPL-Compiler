cmake_minimum_required(VERSION 3.16)

project(MIPS assembler)
set(CMAKE_CXX_STANDARD 17)
add_executable(assembler main.cpp assembler.cpp assembler.h)
