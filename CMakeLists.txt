cmake_minimum_required (VERSION 3.16)

project(Interpreter)

include_directories("${PROJECT_SOURCE_DIR}/Source")
# set(CMAKE_BUILD_TYPE Release)

add_executable(interpreter Source/main.cpp Source/Parser.cpp)

target_compile_options(
    interpreter
    PUBLIC
    -std=c++20
    -O2
    -Wall
    -Werror
    -Wextra
    -Wshadow
    -Wnon-virtual-dtor
    -Wno-old-style-cast
    -Wcast-align
    -Wunused
    -Woverloaded-virtual
    -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wdouble-promotion
    -Wformat=2
    -Wno-unused-variable
    -Wno-ignored-attributes
    -pedantic
)
