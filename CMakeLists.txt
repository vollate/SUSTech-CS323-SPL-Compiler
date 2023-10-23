cmake_minimum_required(VERSION 3.16)

project(SPL-Compiler)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_PRECOMPILE_OUTPUT_DIR ${CMAKE_BINARY_DIR}/pre_compile)

enable_testing()
include_directories(include)
include_directories(${CMAKE_PRECOMPILE_OUTPUT_DIR})
add_subdirectory(src)
add_subdirectory(test)
