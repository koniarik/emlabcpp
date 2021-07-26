cmake_minimum_required(VERSION 3.10.2)

project(emlabcpp_examples)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(
  -g
  -Werror
  -Wextra
  -Wpedantic
  -Wnon-virtual-dtor
  -Wold-style-cast
  -Wcast-align
  -Wunused
  -Woverloaded-virtual
  -Wnull-dereference
  -Wformat=2
  #-Wduplicated-cond
  #-Wlogical-op
  #-Wuseless-cast
  -Wunreachable-code
  -Wsign-conversion
  -Wconversion
  -Wdouble-promotion
  -fmax-errors=5)

include_directories(../include/)

add_executable(algorithm algorithm.cpp)
