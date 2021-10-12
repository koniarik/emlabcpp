cmake_minimum_required(VERSION 3.16)

project(emlabcpp)

include(CTest)

add_library(emlabcpp INTERFACE)
target_include_directories(emlabcpp INTERFACE include/)

if(BUILD_TESTING)
	include(tests/tests.cmake)
endif()

install(TARGETS emlabcpp)
