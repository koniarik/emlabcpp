#include "emlabcpp/static_vector.h"

#pragma once

namespace emlabcpp
{
class testing_record
{
        using arg_buffer = static_vector< uint8_t, 16 >;

        arg_buffer get_binary_arg() const
        {
        }

        template < typename T >
        void log( const T& )
        {
        }

        void fail()
        {
        }

        void success()
        {
        }
};
}  // namespace emlabcpp
