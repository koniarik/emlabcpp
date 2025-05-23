/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
///

#pragma once

#include "../experimental/bounded_view.h"
#include "../range.h"
#include "./base.h"

#include <bit>
#include <span>

namespace emlabcpp::protocol
{

template < base_type T, std::endian Endianess >
struct serializer
{
        static constexpr std::size_t max_size = sizeof( T );
        using size_type                       = bounded< std::size_t, max_size, max_size >;
        static constexpr bool is_big_endian   = Endianess == std::endian::big;

        static constexpr auto& bget( auto& buffer, std::size_t const i )
        {
                return buffer[is_big_endian ? i : max_size - 1 - i];
        }

        static constexpr void serialize_at( std::span< std::byte, max_size > buffer, T item )
        {
                for ( std::size_t const i : range( max_size ) ) {
                        bget( buffer, max_size - i - 1 ) = static_cast< std::byte >( item & 0xFF );
                        item                             = static_cast< T >( item >> 8 );
                }
        }

        static constexpr T deserialize( std::span< std::byte const, max_size > const& buffer )
        {
                T res{};
                for ( std::size_t const i : range( max_size ) ) {
                        res = static_cast< T >( res << 8 );
                        res = static_cast< T >( res | std::to_integer< T >( bget( buffer, i ) ) );
                }
                return { res };
        }
};

template < base_type T, std::endian Endianess >
requires( std::is_enum_v< T > )
struct serializer< T, Endianess >
{
        using utype                           = std::underlying_type_t< T >;
        using userializer                     = serializer< utype, Endianess >;
        static constexpr std::size_t max_size = userializer::max_size;
        using size_type                       = bounded< std::size_t, max_size, max_size >;

        static constexpr void serialize_at( std::span< std::byte, max_size > buffer, T item )
        {
                userializer::serialize_at( buffer, static_cast< utype >( item ) );
        }

        static constexpr T deserialize( std::span< std::byte const, max_size > const& buffer )
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
        serialize_at( std::span< std::byte, max_size > buffer, float const item )
        {
                auto const v = std::bit_cast< uint32_t >( item );
                sub_serializer::serialize_at( buffer, v );
        }

        static constexpr float deserialize( std::span< std::byte const, max_size > const& buffer )
        {
                uint32_t const v = sub_serializer::deserialize( buffer );
                return std::bit_cast< float >( v );
        }
};

template < std::endian Endianess >
struct serializer< bool, Endianess >
{
        static constexpr std::size_t max_size = sizeof( bool );
        using size_type                       = bounded< std::size_t, max_size, max_size >;

        static constexpr void
        serialize_at( std::span< std::byte, max_size > const buffer, bool const v )
        {
                buffer[0] = v ? std::byte{ 0x1 } : std::byte{ 0x0 };
        }

        static constexpr bool deserialize( std::span< std::byte const, max_size > const& buffer )
        {
                return buffer[0] == std::byte{ 0x1 };
        }
};

}  // namespace emlabcpp::protocol
