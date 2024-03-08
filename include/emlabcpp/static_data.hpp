#pragma once

#define EMLABCPP_STATIC_DATA( variable_name, data_infix )                             \
        extern "C" const char _binary_##data_infix##_start;                           \
        extern "C" const char _binary_##data_infix##_end;                             \
                                                                                      \
        static std::string_view variable_name                                         \
        {                                                                             \
                &_binary_##data_infix##_start,                                        \
                    static_cast< std::size_t >(                                       \
                        &_binary_##data_infix##_end - &_binary_##data_infix##_start ) \
        }
