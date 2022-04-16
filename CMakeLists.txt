cmake_minimum_required(VERSION 3.16)

option(EMLABCPP_TESTS_ENABLED "Decides whenever tests should be enabled" OFF)

project(emlabcpp)

set(CMAKE_CXX_STANDARD 20)

add_library(emlabcpp STATIC
    src/experimental/testing/reactor.cpp
    src/experimental/testing/record.cpp
    src/experimental/testing/reactor_interface_adapter.cpp
    src/experimental/testing/controller.cpp
    )
target_include_directories(emlabcpp PUBLIC include/)
target_include_directories(emlabcpp PRIVATE src/)
target_compile_options(emlabcpp PRIVATE
        -Wall -Wextra -pedantic -Wconversion -Wnon-virtual-dtor
    )

if(EMLABCPP_TESTS_ENABLED)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
        add_subdirectory(examples)
    endif()
endif()

install(TARGETS emlabcpp)
