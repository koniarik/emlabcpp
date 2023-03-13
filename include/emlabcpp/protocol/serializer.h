///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
/// associated documentation files (the "Software"), to deal in the Software without restriction,
/// including without limitation the rights to use, copy, modify, merge, publish, distribute,
/// sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or substantial
/// portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
/// NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
/// OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
/// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#include "emlabcpp/experimental/bounded_view.h"
#include "emlabcpp/protocol/base.h"
#include "emlabcpp/range.h"

#include <bit>
#include <span>

#pragma once

namespace emlabcpp::protocol
{

template < base_type T, std::endian Endianess >
struct serializer
{
        static constexpr std::size_t max_size = sizeof( T );
        using size_type                       = bounded< std::size_t, max_size, max_size >;
        static constexpr bool is_big_endian   = Endianess == std::endian::big;

        static constexpr auto& bget( auto& buffer, const std::size_t i )
        {
                return buffer[is_big_endian ? i : max_size - 1 - i];
        }
        static constexpr void serialize_at( std::span< uint8_t, max_size > buffer, T item )
        {
                for ( const std::size_t i : range( max_size ) ) {
                        bget( buffer, max_size - i - 1 ) = static_cast< uint8_t >( item & 0xFF );
                        item                             = static_cast< T >( item >> 8 );
                }
        }
        static constexpr T deserialize( const std::span< const uint8_t, max_size >& buffer )
        {
                T res{};
                for ( std::size_t i : range( max_size ) ) {
                        res = static_cast< T >( res << 8 );
                        res = static_cast< T >( res | bget( buffer, i ) );
                }
                return { res };
        }
};

template < base_type T, std::endian Endianess >
requires( std::is_enum_v< T > ) struct serializer< T, Endianess >
{
        using utype                           = std::underlying_type_t< T >;
        using userializer                     = serializer< utype, Endianess >;
        static constexpr std::size_t max_size = userializer::max_size;
        using size_type                       = bounded< std::size_t, max_size, max_size >;

        static constexpr void serialize_at( std::span< uint8_t, max_size > buffer, T item )
        {
                userializer::serialize_at( buffer, static_cast< utype >( item ) );
        }
        static constexpr T deserialize( const std::span< const uint8_t, max_size >& buffer )
        {
                return static_cast< T >( userializer::deserialize( buffer ) );
        }
};

template < std::endian Endianess >
struct serializer< float, Endianess >
{
        using sub_serializer = serializer< uint32_t, Endianess >;
        static_assert( sizeof( float ) == sizeof( uint32_t ) );

        static constexpr std::size_t max_size = sizeof( float );
        using size_type                       = bounded< std::size_t, max_size, max_size >;

        static constexpr void
        serialize_at( std::span< uint8_t, max_size > buffer, const float item )
        {
                uint32_t v = 0;
                std::memcpy( &v, &item, sizeof( float ) );
                sub_serializer::serialize_at( buffer, v );
        }

        static constexpr float deserialize( const std::span< const uint8_t, max_size >& buffer )
        {
                const uint32_t v   = sub_serializer::deserialize( buffer );
                float          res = 0;
                std::memcpy( &res, &v, sizeof( float ) );
                return res;
        }
};

template < std::endian Endianess >
struct serializer< bool, Endianess >
{
        static constexpr std::size_t max_size = sizeof( bool );
        using size_type                       = bounded< std::size_t, max_size, max_size >;

        static constexpr void
        serialize_at( const std::span< uint8_t, max_size > buffer, const bool v )
        {
                buffer[0] = v ? 0x1 : 0x0;
        }
        static constexpr bool deserialize( const std::span< const uint8_t, max_size >& buffer )
        {
                return buffer[0] == 0x1;
        }
};

}  // namespace emlabcpp::protocol
