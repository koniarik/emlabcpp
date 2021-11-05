cmake_minimum_required(VERSION 3.16)

project(emlabcpp)


add_library(emlabcpp INTERFACE)
target_include_directories(emlabcpp INTERFACE include/)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	include(CTest)
	if(BUILD_TESTING)
		include(tests/tests.cmake)
	endif()
endif()

install(TARGETS emlabcpp)
