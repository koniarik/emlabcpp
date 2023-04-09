#
# Copyright (C) 2020 Jan Veverak Koniarik
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

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
            -Wdouble-promotion
            -Wno-mismatched-new-delete
            -Wno-format-nonliteral)
endfunction()
