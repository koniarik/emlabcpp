
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(add_emlabcpp_example name)
	add_executable(${name}_example ${name}.cpp)
	target_link_libraries(${name}_example emlabcpp)
  
  target_compile_options(${name}_example PRIVATE
		-DEMLABCPP_USE_STREAMS
  	-DEMLABCPP_ASSERT_NATIVE
	)

	add_test(NAME ${name}_example COMMAND ${name}_example)
endfunction()

add_emlabcpp_example(algorithm)
add_emlabcpp_example(protocol)

