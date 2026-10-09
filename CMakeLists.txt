cmake_minimum_required(VERSION 3.23)

project(emlabcpp VERSION 2.0.0)

option(EMLABCPP_COVERAGE_ENABLED
       "Decides whenever the corevage should be builded" OFF)
option(EMLABCPP_TESTS_ENABLED "Decides whenever tests should be enabled" OFF)
# PROJECT_IS_TOP_LEVEL is set by project(), so this has to stay below it.
option(EMLABCPP_INSTALL "Generate install rules" ${PROJECT_IS_TOP_LEVEL})

include(cmake/util.cmake)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(emlabcpp INTERFACE)
add_library(emlabcpp::emlabcpp ALIAS emlabcpp)
target_compile_features(emlabcpp INTERFACE cxx_std_23)

file(GLOB_RECURSE EMLABCPP_INCLUDE_FILES "include/*.hpp")

target_sources(
  emlabcpp
  PUBLIC FILE_SET
         public_headers
         TYPE
         HEADERS
         BASE_DIRS
         include
         FILES
         ${EMLABCPP_INCLUDE_FILES})

if(EMLABCPP_COVERAGE_ENABLED)
  emlabcpp_target_enable_coverage(emlabcpp)
endif()

if(EMLABCPP_TESTS_ENABLED)
  if(NOT BUILD_TESTING STREQUAL OFF)
    include(CTest)
    add_subdirectory(tests)
    add_subdirectory(examples)
  endif()
endif()

if(EMLABCPP_INSTALL)
  install(
    TARGETS emlabcpp
    EXPORT emlabcpp
    FILE_SET public_headers)
  install(
    EXPORT emlabcpp
    FILE emlabcpp-config.cmake
    NAMESPACE emlabcpp::
    DESTINATION lib/cmake/emlabcpp)
  include(CMakePackageConfigHelpers)
  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/emlabcpp-config-version.cmake
    COMPATIBILITY SameMajorVersion ARCH_INDEPENDENT)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/emlabcpp-config-version.cmake
          DESTINATION lib/cmake/emlabcpp)
endif()
