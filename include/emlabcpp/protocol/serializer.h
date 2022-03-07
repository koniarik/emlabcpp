#include "emlabcpp/bounded_view.h"
#include "emlabcpp/iterators/numeric.h"
#include "emlabcpp/protocol/base.h"

#include <span>

#pragma once

namespace emlabcpp
{

template < protocol_base_type T, protocol_endianess_enum Endianess >
struct protocol_serializer
{
        static constexpr std::size_t max_size = sizeof( T );
        using size_type                       = bounded< std::size_t, max_size, max_size >;
        using view_type                       = bounded_view< const uint8_t*, size_type >;
        static constexpr bool is_big_endian   = Endianess == PROTOCOL_BIG_ENDIAN;

        static constexpr auto& bget( auto& buffer, std::size_t i )
        {
                return buffer[is_big_endian ? i : max_size - 1 - i];
        }
        static constexpr void serialize_at( std::span< uint8_t, max_size > buffer, T item )
        {
                for ( std::size_t i : range( max_size ) ) {
                        bget( buffer, max_size - i - 1 ) = static_cast< uint8_t >( item & 0xFF );
                        item                             = static_cast< T >( item >> 8 );
                }
        }
        static constexpr T deserialize( const view_type& buffer )
        {
                T res{};
                for ( std::size_t i : range( max_size ) ) {
                        res = static_cast< T >( res << 8 );
                        res = static_cast< T >( res | bget( buffer, i ) );
                }
                return { res };
        }
};

}  // namespace emlabcpp
