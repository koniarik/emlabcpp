cmake_minimum_required(VERSION 3.10.2)

project(emlabcpp)

enable_testing()

add_subdirectory(3rd_party/google-test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(
  -gdwarf
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
  -fmax-errors=5
  -DEMLABCPP_USE_STREAMS
  -DEMLABCPP_ASSERT_NATIVE
  )

# TODO: this should use top level cmake
include_directories(../include/)

function(add_emlabcpp_test name)
  add_executable(${name} ${name}.cpp)
  target_link_libraries(${name} GTest::GTest GTest::Main)
  target_include_directories(${name} PRIVATE ../tests/include/)
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
add_emlabcpp_test(protocol_item_test)
add_emlabcpp_test(protocol_sophisticated_test)
add_emlabcpp_test(protocol_register_map_test)
