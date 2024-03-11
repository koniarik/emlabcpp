include(GoogleTest)

find_package(GTest REQUIRED CONFIG)

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

function(emlabcpp_setup_test target)
  target_include_directories(${target} PRIVATE include/)
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

function(add_emlabcpp_test name)
  add_executable(${name} ${name}.cpp)
  target_link_libraries(${name} GTest::gtest GTest::gtest_main)
  emlabcpp_setup_test(${name})
endfunction()

add_emlabcpp_test(algorithm_test)
add_emlabcpp_test(min_max_test)
add_emlabcpp_test(bounded_test)
add_emlabcpp_test(concepts_test)
add_emlabcpp_test(decompose_test)
add_emlabcpp_test(either_test)
add_emlabcpp_test(function_view_test)
add_emlabcpp_test(linked_list_test)
add_emlabcpp_test(match_test)
add_emlabcpp_test(matrix_test)
add_emlabcpp_test(numeric_iterator_test)
add_emlabcpp_test(physical_quantity_test)
add_emlabcpp_test(pid_test)
add_emlabcpp_test(pmr_test)
add_emlabcpp_test(pretty_printer_test)
add_emlabcpp_test(protocol_def_test)
add_emlabcpp_test(protocol_packet_test)
add_emlabcpp_test(protocol_register_map_test)
add_emlabcpp_test(protocol_seq_test)
add_emlabcpp_test(protocol_sophisticated_test)
add_emlabcpp_test(request_reply_test)
add_emlabcpp_test(rpc_test)
add_emlabcpp_test(static_circular_buffer_test)
add_emlabcpp_test(static_function_test)
add_emlabcpp_test(static_vector_test)
add_emlabcpp_test(testing_collect_test)
add_emlabcpp_test(testing_awaiters_test)
add_emlabcpp_test(testing_combined_test)
add_emlabcpp_test(testing_json)
add_emlabcpp_test(testing_params_test)
add_emlabcpp_test(testing_reactor_test)
add_emlabcpp_test(types_test)
add_emlabcpp_test(visit_test)
add_emlabcpp_test(zip_test)
add_emlabcpp_test(line_test)
add_emlabcpp_test(point_test)
add_emlabcpp_test(pose_test)
add_emlabcpp_test(quaternion_test)
add_emlabcpp_test(simplex_test)
add_emlabcpp_test(cfg_test)
add_emlabcpp_test(cobs_test)
