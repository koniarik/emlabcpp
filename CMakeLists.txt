cmake_minimum_required(VERSION 3.16)

project(emlabcpp)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

add_library(emlabcpp INTERFACE)
target_include_directories(emlabcpp INTERFACE include/)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(examples)
endif()

install(TARGETS emlabcpp)
