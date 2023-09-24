macro(emlabcpp_add_resource target resource)
  # NOTE: stolen from a blogpost which I forgot to link /o\

  add_custom_command(
    OUTPUT ${target}.o
    COMMAND ld -r -b binary -o ${CMAKE_CURRENT_BINARY_DIR}/${target}.o
            ${resource}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${resource}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Building resource ${resource}"
    VERBATIM)

  add_library(${target} STATIC ${target}.o)
  set_target_properties(${target} PROPERTIES LINKER_LANGUAGE CXX)

endmacro()
