find_program(
  EMLABCPP_CLANG_FORMAT_EXECUTABLE
  NAMES clang-format
        clang-format-13
        clang-format-12
        clang-format-11
        clang-format-10
        clang-format-9
        clang-format-8
        clang-format-7
        REQUIRED
  DOC "clang-format executable")

function(emlabcpp_add_format_test)
  cmake_parse_arguments(A "" "TARGET;WORKING_DIRECTORY" "FILES" ${ARGN})
  add_test(
    NAME ${A_TARGET}
    COMMAND ${EMLABCPP_CLANG_FORMAT_EXECUTABLE} --dry-run -Werror ${A_FILES}
    WORKING_DIRECTORY ${A_WORKING_DIRECTORY})
endfunction()

function(emlabcpp_target_enable_coverage target)
  target_compile_options(${target} PRIVATE --coverage)
  target_link_libraries(${target} gcov)
endfunction()

function(emlabcpp_compile_options target)
  target_compile_options(
    ${target}
    PRIVATE -Wall
            -Wextra
            -pedantic
            -Wconversion
            -Wnon-virtual-dtor
            -Wold-style-cast
            -Wcast-align
            -Wunused
            -Woverloaded-virtual
            -Wunreachable-code
            -Wdouble-promotion)
endfunction()
