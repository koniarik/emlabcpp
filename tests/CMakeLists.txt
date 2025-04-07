include(GoogleTest)

include(third-party/CPM.cmake)

cpmaddpackage(
  NAME
  googletest
  GITHUB_REPOSITORY
  google/googletest
  GIT_TAG
  v1.16.0
  VERSION
  1.16.0
  OPTIONS
  "INSTALL_GTEST OFF"
  "gtest_force_shared_crt")
cpmaddpackage("gh:nlohmann/json@3.11.3")

target_compile_options(
  emlabcpp
  PUBLIC -DEMLABCPP_USE_NLOHMANN_JSON
         -DEMLABCPP_ASSERT_NATIVE
         -DEMLABCPP_USE_MAGIC_ENUM
         -DEMLABCPP_USE_GTEST
         -DEMLABCPP_USE_LOGGING
         -DEMLABCPP_USE_DEMANGLING
         -DEMLABCPP_USE_OSTREAM)
target_include_directories(emlabcpp SYSTEM PUBLIC third-party/)
target_link_libraries(emlabcpp PUBLIC nlohmann_json)

function(emlabcpp_setup_test target)
  target_link_libraries(${target} emlabcpp)
  # gtest_discover_tests(${target})
  add_test(NAME ${target}_test COMMAND ${target})

  target_compile_options(
    ${target}
    PRIVATE -ggdb
            -Wall
            -Wextra
            -Wpedantic
            -Wnon-virtual-dtor
            -Wold-style-cast
            -Wcast-align
            -Wunused
            -Woverloaded-virtual
            -Wnull-dereference
            -Wformat=2
            -Wunreachable-code
            -Wsign-conversion
            -Wconversion
            -Wdouble-promotion
            -ftemplate-backtrace-limit=0
            -Wno-mismatched-new-delete
            -DEMLABCPP_USE_LOGGING
            -DEMLABCPP_USE_DEMANGLING
            -DEMLABCPP_ASSERT_NATIVE
            -DEMLABCPP_USE_NLOHMANN_JSON
            -DEMLABCPP_USE_MAGIC_ENUM
            -DEMLABCPP_USE_GTEST)
  if(EMLABCPP_COVERAGE_ENABLED)
    emlabcpp_target_enable_coverage(${target})
  endif()
endfunction()

file(GLOB TESTS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(emlabcpp_tests ${TESTS} util/operations_counter.cpp)
target_link_libraries(emlabcpp_tests gtest GTest::gtest_main)
emlabcpp_setup_test(emlabcpp_tests)
