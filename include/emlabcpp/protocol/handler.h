#include "emlabcpp/protocol/item.h"

#pragma once

namespace emlabcpp
{

template < typename T >
struct protocol_handler
{
        using def_type     = typename T::def_type;
        using value_type   = typename T::value_type;
        using message_type = typename T::message_type;

        using pitem = protocol_item< def_type, PROTOCOL_BIG_ENDIAN >;

        static message_type serialize( value_type val )
        {
                std::array< uint8_t, pitem::max_size > buffer;

                bounded used = pitem::serialize_at( buffer, val );

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
