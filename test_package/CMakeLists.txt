cmake_minimum_required(VERSION 3.15)
project(EmlabcppTest CXX)

find_package(emlabcpp CONFIG REQUIRED)

add_executable(example src/example.cpp)
target_link_libraries(example emlabcpp::emlabcpp)
