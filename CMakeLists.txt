cmake_minimum_required(VERSION 3.16)

project(emlabcpp)

add_library(emlabcpp INTERFACE)
target_include_directories(emlabcpp INTERFACE include/)

install(TARGETS emlabcpp)
