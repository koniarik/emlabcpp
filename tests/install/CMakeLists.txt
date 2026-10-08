# Builds against an installed emlabcpp; point CMAKE_PREFIX_PATH at the install
# prefix.
cmake_minimum_required(VERSION 3.23)

project(emlabcpp_install_consumer CXX)

find_package(emlabcpp 2.0 REQUIRED)

add_executable(consumer main.cpp)
target_link_libraries(consumer emlabcpp::emlabcpp)
