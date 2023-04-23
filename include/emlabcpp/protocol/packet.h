///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
/// and associated documentation files (the "Software"), to deal in the Software without
/// restriction, including without limitation the rights to use, copy, modify, merge, publish,
/// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or
/// substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
/// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
/// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#include "emlabcpp/protocol/sequencer.h"
#include "emlabcpp/protocol/serializer.h"
#include "emlabcpp/protocol/tuple.h"

#pragma once

namespace emlabcpp::protocol
{

template < typename T >
concept packet_def = requires( T t )
{
        {
                T::endianess
                } -> std::convertible_to< std::endian >;
        is_std_array_v< std::decay_t< decltype( T::prefix ) > >;
        requires convertible< typename T::size_type >;
        requires convertible< typename T::checksum_type >;
};

template < typename Def, typename Payload >
using packet_base = tuple<
    Def::endianess,
    std::decay_t< decltype( Def::prefix ) >,
    sized_buffer< typename Def::size_type, Payload >,
    typename Def::checksum_type >;

template < packet_def Def, typename Payload >
struct packet : packet_base< Def, Payload >
{
        using payload_type = Payload;
        using base         = packet_base< Def, Payload >;

        static constexpr auto prefix    = Def::prefix;
        static constexpr auto endianess = Def::endianess;

        using prefix_type                        = std::decay_t< decltype( prefix ) >;
        using prefix_traits                      = proto_traits< prefix_type >;
        static constexpr std::size_t prefix_size = prefix_traits::max_size;

        using size_type                        = typename Def::size_type;
        using size_traits                      = proto_traits< size_type >;
        static constexpr std::size_t size_size = size_traits::max_size;

        using payload_traits = proto_traits< Payload >;
        using value_type     = typename payload_traits::value_type;

        using checksum_type   = typename Def::checksum_type;
        using checksum_traits = proto_traits< checksum_type >;

        static_assert( fixedly_sized< size_type > );

        struct sequencer_def
        {
                using message_type    = typename base::message_type;
                using serializer_type = serializer< size_type, endianess >;

                static constexpr auto        prefix     = Def::prefix;
                static constexpr std::size_t fixed_size = prefix_size + size_size;

                static constexpr std::size_t get_size( const auto& buffer )
                {
                        std::array< std::byte, size_size > tmp;
                        std::copy_n( std::begin( buffer ) + prefix_size, size_size, tmp.begin() );
                        return serializer_type::deserialize( tmp ) + fixed_size +
                               checksum_traits::max_size;
                }
        };

        using sequencer_type = sequencer< sequencer_def >;

        static constexpr checksum_type get_checksum( const view< const std::byte* > mview )
        {
                return Def::get_checksum( mview );
        }
};

}  // namespace emlabcpp::protocol
