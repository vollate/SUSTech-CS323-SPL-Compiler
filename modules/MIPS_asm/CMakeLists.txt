cmake_minimum_required(VERSION 3.14)

project(MIPS_assembler)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_executable(splc ./src/utils.cpp ./src/main.cpp ./src/mips32.cpp)
include_directories(include)
if(CMAKE_BUILD_TYPE MATCHES Debug)
  add_compile_definitions(SPLC_DEBUG)
endif()

install(TARGETS splc DESTINATION "${CMAKE_SOURCE_DIR}/bin")
