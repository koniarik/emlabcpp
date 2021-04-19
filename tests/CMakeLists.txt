cmake_minimum_required(VERSION 3.10.2)

project(emlabcpp)

enable_testing()

add_subdirectory(3rd_party/google-test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
  -fmax-errors=15)

include_directories(../include/)

function(add_emlabcpp_test name)
  add_executable(${name} ${name}.cpp)
  target_link_libraries(${name} GTest::GTest GTest::Main)
  add_test(NAME ${name} COMMAND ${name})
endfunction()

add_emlabcpp_test(static_circular_buffer_test)
add_emlabcpp_test(algorithm_test)
add_emlabcpp_test(numeric_iterator_test)
add_emlabcpp_test(access_iterator_test)
add_emlabcpp_test(either_test)
add_emlabcpp_test(physical_quantity_test)
add_emlabcpp_test(zip_test)
add_emlabcpp_test(static_vector_test)
add_emlabcpp_test(pid_test)
