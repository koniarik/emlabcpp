cmake_minimum_required(VERSION 3.16)

option(EMLABCPP_TESTS_ENABLED "Decides whenever tests should be enabled" OFF)
option(EMLABCPP_COVERAGE_ENABLED
       "Decides whenever the corevage should be builded" OFF)

project(emlabcpp)

include(cmake/util.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(
  emlabcpp STATIC
  src/experimental/testing/reactor.cpp
  src/experimental/testing/record.cpp
  src/experimental/testing/reactor_interface_adapter.cpp
  src/experimental/testing/protocol.cpp
  src/experimental/logging.cpp
  src/experimental/testing/json.cpp
  src/experimental/testing/controller.cpp)
target_include_directories(emlabcpp PUBLIC include/)
target_include_directories(emlabcpp PRIVATE src/)
target_compile_options(
  emlabcpp
  PRIVATE -Wall
          -Wextra
          -pedantic
          -Wconversion
          -Wnon-virtual-dtor
          -Wold-style-cast
          -Wcast-align
          -Wunused
          -Woverloaded-virtual
          -Wunreachable-code
          -Wdouble-promotion)

if(EMLABCPP_COVERAGE_ENABLED)
  emlabcpp_target_enable_coverage(emlabcpp)
endif()

if(EMLABCPP_TESTS_ENABLED)
  include(CTest)
  if(BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(examples)
  endif()
endif()

install(TARGETS emlabcpp)
