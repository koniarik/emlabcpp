#include "emlabcpp/static_vector.h"

#include <map>
#include <memory_resource>
#include <variant>

#pragma once

namespace emlabcpp
{

using testing_key_buffer  = static_vector< char, 16 >;
using testing_name_buffer = static_vector< char, 32 >;

using testing_key           = std::variant< uint32_t, testing_key_buffer >;
using testing_string_buffer = static_vector< char, 32 >;
using testing_arg_variant   = std::variant< uint64_t, int64_t, bool, testing_string_buffer >;
using testing_run_id        = uint32_t;
using testing_test_id       = uint16_t;

// TODO: maybe make a function in static_vector namespace?
template < typename T >
inline T testing_string_to_buffer( std::string_view sview )
{
        T tmp;
        std::copy_n( sview.begin(), std::min( sview.size(), T::capacity ), std::back_inserter( tmp ) );
        return tmp;
}

inline testing_name_buffer testing_name_to_buffer( std::string_view sview )
{
        return testing_string_to_buffer< testing_name_buffer >( sview );
}

inline testing_key_buffer testing_key_to_buffer( std::string_view key )
{
        return testing_string_to_buffer< testing_key_buffer >( key );
}

inline testing_string_buffer testing_string_to_buffer( std::string_view st )
{
        return testing_string_to_buffer< testing_string_buffer >( st );
}

struct testing_result
{
        testing_test_id                                   tid;
        testing_run_id                                    rid;
        std::pmr::map< testing_key, testing_arg_variant > collected_data;
        bool                                              failed  = false;
        bool                                              errored = false;
};

class testing_reactor_interface_adapter;

}  // namespace emlabcpp
