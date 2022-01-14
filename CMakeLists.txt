cmake_minimum_required(VERSION 3.16)

option(EMLABCPP_TESTS_ENABLED "Decides whenever tests should be enabled" OFF)

project(emlabcpp)

add_library(emlabcpp INTERFACE)
target_include_directories(emlabcpp INTERFACE include/)

if(EMLABCPP_TESTS_ENABLED)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
        add_subdirectory(examples)
    endif()
endif()

install(TARGETS emlabcpp)
