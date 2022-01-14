cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED PROJECT_NAME) # detection for disabling test inspired by Catch2
  set(NOT_SUBPROJECT ON)
else()
  set(NOT_SUBPROJECT OFF)
endif()

project(emlabcpp)

add_library(emlabcpp INTERFACE)
target_include_directories(emlabcpp INTERFACE include/)

if(NOT_SUBPROJECT)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
        add_subdirectory(examples)
    endif()
endif()

install(TARGETS emlabcpp)
