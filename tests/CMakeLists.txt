cmake_minimum_required(VERSION 3.10.2)

project(emlabcpp)

enable_testing()

add_subdirectory(3rd_party/google-test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(
	-Werror -Wextra -Wpedantic -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wnull-dereference -Wformat=2 -Wduplicated-cond -Wlogical-op -Wuseless-cast -Wunreachable-code -Wsign-conversion -Wconversion -Wdouble-promotion
	)

include_directories(../)

add_executable(static_circular_buffer_test static_circular_buffer_test.cpp)
target_link_libraries(static_circular_buffer_test GTest::GTest GTest::Main)
add_test(NAME static_circular_buffer_test COMMAND static_circular_buffer_test)

