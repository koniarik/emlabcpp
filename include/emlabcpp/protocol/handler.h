#include "emlabcpp/protocol/def.h"

#pragma once

namespace emlabcpp
{

// protocol_handler< T > should be used to execute actual serialization and deserealization of
// protocol definition. It provides serialize/extract methods that should be used by the user.
//
// You may want to have this class (With the include) to be present in separate .cpp file, as the
// compile time can be quite heavy.
template < typename T >
struct protocol_handler
{
        using decl         = protocol_def< T, PROTOCOL_BIG_ENDIAN >;
        using value_type   = typename decl::value_type;
        using message_type = protocol_message< decl::max_size >;

        static message_type serialize( value_type val )
        {
                std::array< uint8_t, decl::max_size > buffer{};

                bounded used = decl::serialize_at( buffer, val );
                EMLABCPP_ASSERT( *used <= decl::max_size );
                return *message_type::make( view_n( buffer.begin(), *used ) );
        };

        static either< value_type, protocol_error_record > extract( const message_type& msg )
        {
                return decl::deserialize( msg ).convert_left( [&]( auto sub_res ) {
                        return sub_res.val;
                } );
        }
};

}  // namespace emlabcpp
