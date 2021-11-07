include(cmake/util.cmake)

add_subdirectory(tests/3rd_party/google-test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(emlabcpp_setup_test target)
  target_include_directories(${target} PRIVATE tests/include/)
  target_link_libraries(${target} emlabcpp)
  add_test(NAME ${target} COMMAND ${target})

  target_compile_options( ${target} PRIVATE
  -gdwarf
  -Werror
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
  -fmax-errors=5
  -fconcepts-diagnostics-depth=5
  -DEMLABCPP_USE_STREAMS
  -DEMLABCPP_ASSERT_NATIVE
  )

endfunction()

function(add_emlabcpp_test name)
  add_executable(${name} tests/${name}.cpp)
  target_link_libraries(${name} GTest::GTest GTest::Main)
  emlabcpp_setup_test(${name})
endfunction()

add_emlabcpp_test(static_circular_buffer_test)
add_emlabcpp_test(algorithm_test)
add_emlabcpp_test(numeric_iterator_test)
add_emlabcpp_test(access_iterator_test)
add_emlabcpp_test(either_test)
add_emlabcpp_test(physical_quantity_test)
add_emlabcpp_test(zip_test)
add_emlabcpp_test(static_vector_test)
add_emlabcpp_test(pid_test)
add_emlabcpp_test(protocol_def_test)
add_emlabcpp_test(protocol_sophisticated_test)
add_emlabcpp_test(protocol_register_map_test)
	
include(examples/examples.cmake)

#file(GLOB_RECURSE HEADER_FILES
#		"${PROJECT_SOURCE_DIR}/include/*.h"
#)
#add_format_test(
#		TARGET emlabcpp_format
#		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
#		FILES ${HEADER_FILES})