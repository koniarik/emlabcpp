#include "emlabcpp/protocol/item.h"

#pragma once

namespace emlabcpp
{

template < typename T >
struct protocol_handler
{
        using pitem        = protocol_item< T, PROTOCOL_BIG_ENDIAN >;
        using value_type   = typename pitem::value_type;
        using message_type = protocol_message< pitem::max_size >;

        static message_type serialize( value_type val )
        {
                std::array< uint8_t, pitem::max_size > buffer;

                bounded used = pitem::serialize_at( buffer, val );
                EMLABCPP_ASSERT( *used <= pitem::max_size );
                return *message_type::make( view_n( buffer.begin(), *used ) );
        };

        static either< value_type, protocol_error_record > extract( const message_type& msg )
        {
                return pitem::deserialize( msg ).convert_left( [&]( auto sub_res ) {
                        return sub_res.val;
                } );
        }
};

}  // namespace emlabcpp
