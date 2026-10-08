function(add_emlabcpp_example name)
  add_executable(${name}_example ${name}.cpp)
  target_link_libraries(${name}_example emlabcpp)

  target_compile_options(${name}_example PRIVATE -DEMLABCPP_USE_OSTREAM
                                                 -DEMLABCPP_ASSERT_NATIVE)

  add_test(NAME ${name}_example COMMAND ${name}_example)
endfunction()

add_emlabcpp_example(algorithm)
add_emlabcpp_example(protocol)
