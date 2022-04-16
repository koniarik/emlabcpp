find_program(
  CLANG_FORMAT_EXECUTABLE
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

function(add_format_test)
  cmake_parse_arguments(A "" "TARGET;WORKING_DIRECTORY" "FILES" ${ARGN})
  add_test(
    NAME ${A_TARGET}
    COMMAND ${CLANG_FORMAT_EXECUTABLE} --dry-run -Werror ${A_FILES}
    WORKING_DIRECTORY ${A_WORKING_DIRECTORY})
endfunction()
