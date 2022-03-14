#include "emlabcpp/static_vector.h"

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

inline testing_name_buffer testing_make_name_buffer( std::string_view sview )
{
        // TODO: not thrilled about this, maybe design this better? so that the assert is not needed
        // and hence this can't fail?
        EMLABCPP_ASSERT( sview.size() <= testing_name_buffer::capacity );

        testing_name_buffer res{};
        std::copy( sview.begin(), sview.end(), std::back_inserter( res ) );
        return res;
}

inline testing_key_buffer testing_key_to_buffer( std::string_view key )
{
        testing_key_buffer tmp;
        std::copy_n(
            key.begin(),
            min( key.size(), testing_key_buffer::capacity ),
            std::back_inserter( tmp ) );
        return tmp;
}

}  // namespace emlabcpp
