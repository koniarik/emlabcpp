cmake_minimum_required(VERSION 3.23)

option(EMLABCPP_COVERAGE_ENABLED
       "Decides whenever the corevage should be builded" OFF)
option(EMLABCPP_TESTS_ENABLED "Decides whenever tests should be enabled" OFF)

project(emlabcpp)

include(cmake/util.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(
  emlabcpp STATIC
  src/experimental/testing/reactor.cpp
  src/experimental/testing/reactor_interface_adapter.cpp
  src/experimental/testing/protocol.cpp
  src/experimental/testing/parameters.cpp
  src/experimental/testing/collect.cpp
  src/experimental/testing/interface.cpp
  src/experimental/logging.cpp
  src/experimental/testing/json.cpp
  src/experimental/testing/controller.cpp)
target_include_directories(emlabcpp PUBLIC include/)
target_include_directories(emlabcpp PRIVATE src/)
emlabcpp_compile_options(emlabcpp)

file(GLOB_RECURSE EMLABCPP_INCLUDE_FILES "include/*.h")

target_sources(
  emlabcpp
  PUBLIC FILE_SET
         public_headers
         TYPE
         HEADERS
         BASE_DIRS
         include
         FILES
         ${EMLABCPP_INCLUDE_FILES})

if(EMLABCPP_COVERAGE_ENABLED)
  emlabcpp_target_enable_coverage(emlabcpp)
endif()

if(EMLABCPP_TESTS_ENABLED)
  if(NOT BUILD_TESTING STREQUAL OFF)
    include(CTest)
    add_subdirectory(tests)
    add_subdirectory(examples)
  endif()
endif()

install(TARGETS emlabcpp FILE_SET public_headers)
