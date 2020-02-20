cmake_minimum_required(VERSION 3.13)

project(emlabcpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_subdirectory(3rd_party/google-test)

include_directories(../)

add_executable(static_circular_buffer_test static_circular_buffer_test.cpp)
target_link_libraries(static_circular_buffer_test GTest::GTest GTest::Main)
add_test(NAME static_circular_buffer_test COMMAND static_circular_buffer_Test)

