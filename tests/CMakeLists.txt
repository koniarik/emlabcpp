# SYSTEM of FetchContent_Declare needs CMake 3.25.
cmake_minimum_required(VERSION 3.25)

include(GoogleTest)
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.18.0
  GIT_SHALLOW TRUE
  SYSTEM)
set(INSTALL_GTEST
    OFF
    CACHE BOOL "")
set(gtest_force_shared_crt
    ON
    CACHE BOOL "")
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
  GIT_SHALLOW TRUE
  SYSTEM)
FetchContent_Declare(
  magic_enum
  GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
  GIT_TAG v0.9.8
  GIT_SHALLOW TRUE
  SYSTEM)
FetchContent_MakeAvailable(googletest nlohmann_json magic_enum)

# MSVC's standard library has no <cxxabi.h>; its typeid names are readable.
include(CheckIncludeFileCXX)
check_include_file_cxx(cxxabi.h EMLABCPP_HAVE_CXXABI)
if(EMLABCPP_HAVE_CXXABI)
  set(EMLABCPP_TYPE_NAMES -DEMLABCPP_USE_DEMANGLING)
else()
  set(EMLABCPP_TYPE_NAMES -DEMLABCPP_USE_TYPEID)
endif()

add_library(emlabcpp_test_options INTERFACE)
target_compile_options(
  emlabcpp_test_options
  INTERFACE -DEMLABCPP_USE_NLOHMANN_JSON
            -DEMLABCPP_ASSERT_NATIVE
            # -DEMLABCPP_USE_MAGIC_ENUM # dropped until clang issues resolved
            -DEMLABCPP_USE_GTEST
            -DEMLABCPP_USE_LOGGING
            ${EMLABCPP_TYPE_NAMES}
            -DEMLABCPP_USE_OSTREAM)
target_link_libraries(emlabcpp_test_options INTERFACE nlohmann_json
                                                      magic_enum::magic_enum)
# BUILD_INTERFACE keeps the test options out of the installed package.
target_link_libraries(emlabcpp
                      INTERFACE $<BUILD_INTERFACE:emlabcpp_test_options>)

function(emlabcpp_setup_test target)
  target_link_libraries(${target} emlabcpp)
  # gtest_discover_tests(${target})
  add_test(NAME ${target}_test COMMAND ${target})

  target_compile_options(
    ${target}
    PRIVATE -ggdb
            -Wall
            -Wextra
            -Wpedantic
            -Wnon-virtual-dtor
            -Wcast-align
            -Wunused
            -Woverloaded-virtual
            -Wnull-dereference
            -Wformat=2
            -Wunreachable-code
            -Wsign-conversion
            -Wconversion
            -Wdouble-promotion
            -ftemplate-backtrace-limit=0
            -Wno-mismatched-new-delete
            -DEMLABCPP_USE_LOGGING
            ${EMLABCPP_TYPE_NAMES}
            -DEMLABCPP_ASSERT_NATIVE
            -DEMLABCPP_USE_NLOHMANN_JSON
            -DEMLABCPP_USE_MAGIC_ENUM
            -DEMLABCPP_USE_GTEST)
  emlabcpp_compile_options(${target})
  if(EMLABCPP_COVERAGE_ENABLED)
    emlabcpp_target_enable_coverage(${target})
  endif()
endfunction()

file(GLOB TESTS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(emlabcpp_tests ${TESTS} util/operations_counter.cpp)
target_link_libraries(emlabcpp_tests gtest GTest::gtest_main)
emlabcpp_setup_test(emlabcpp_tests)
# AddressSanitizer aborts on impossible allocations unless told to return null,
# which the memory resource tests rely on.
set_tests_properties(
  emlabcpp_tests_test PROPERTIES ENVIRONMENT
                                 ASAN_OPTIONS=allocator_may_return_null=1)

# Compiles each public header on its own; the verify target is not part of ALL,
# so the tests depend on it.
set_property(TARGET emlabcpp PROPERTY VERIFY_INTERFACE_HEADER_SETS ON)
add_dependencies(emlabcpp_tests emlabcpp_verify_interface_header_sets)

find_package(
  Python3
  COMPONENTS Interpreter
  REQUIRED)
add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/readme.cpp
  COMMAND
    ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/gen_readme_cpp.py
    ${CMAKE_CURRENT_SOURCE_DIR}/../README.md ${CMAKE_BINARY_DIR}/readme.cpp
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../README.md
          ${CMAKE_CURRENT_SOURCE_DIR}/gen_readme_cpp.py)
add_executable(readme ${CMAKE_BINARY_DIR}/readme.cpp)
target_link_libraries(readme PUBLIC emlabcpp gtest GTest::gtest_main)
# README snippets declare values only to show how an API is called.
target_compile_options(readme PRIVATE -Wno-unused -Wno-unused-parameter)
add_test(NAME readme COMMAND readme)
