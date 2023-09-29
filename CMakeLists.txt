cmake_minimum_required(VERSION 3.16)

project(SPL-Compiler)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()
include_directories(include)
add_subdirectory(src)
add_subdirectory(test)
