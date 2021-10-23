
function(add_emlabcpp_example name)
	add_executable(${name}_example examples/${name}.cpp)
	emlabcpp_setup_test(${name}_example)
endfunction()

add_emlabcpp_example(algorithm)
add_emlabcpp_example(protocol)

