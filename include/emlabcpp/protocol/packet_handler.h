// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//
//  Copyright © 2022 Jan Veverak Koniarik
//  This file is part of project: emlabcpp
//
#include "emlabcpp/protocol/handler.h"
#include "emlabcpp/protocol/packet.h"

#pragma once

namespace emlabcpp
{

template < typename Packet >
struct protocol_packet_handler
{
        using message_type  = typename Packet::message_type;
        using prefix_type   = typename Packet::prefix_type;
        using payload_type  = typename Packet::payload_type;
        using value_type    = typename Packet::value_type;
        using size_type     = typename Packet::size_type;
        using checksum_type = typename Packet::checksum_type;

        using sub_handler                          = protocol_handler< Packet >;
        static constexpr std::size_t size_offset   = Packet::prefix_decl::max_size;
        static constexpr std::size_t checksum_size = Packet::checksum_decl::max_size;
        static constexpr std::size_t size_size     = Packet::size_decl::max_size;
        static constexpr auto        endianess     = Packet::endianess;

        static message_type serialize( const value_type& val )
        {

                message_type msg = sub_handler::serialize(
                    std::make_tuple( Packet::prefix, size_type{}, val, checksum_type{} ) );

                auto size = static_cast< size_type >(
                    msg.size() - Packet::prefix_decl::max_size - size_size );

                protocol_serializer< size_type, endianess >::serialize_at(
                    std::span< uint8_t, size_size >{ msg.begin() + size_offset, size_size }, size );

                checksum_type chcksm =
                    Packet::get_checksum( view_n( msg.begin(), msg.size() - checksum_size ) );

                protocol_serializer< checksum_type, endianess >::serialize_at(
                    std::span< uint8_t, checksum_size >{ msg.end() - checksum_size, checksum_size },
                    chcksm );

                return msg;
        }

        static either< value_type, protocol_error_record >
        extract( const view< const uint8_t* >& msg )
        {
                return sub_handler::extract( msg ).bind_left(
                    [&]( std::tuple< prefix_type, size_type, value_type, checksum_type > pack )
                        -> either< value_type, protocol_error_record > {
                            checksum_type present_checksum = std::get< 3 >( pack );
                            std::size_t   checksum_pos     = msg.size() - checksum_size;
                            checksum_type calculated_checksum =
                                Packet::get_checksum( view_n( msg.begin(), checksum_pos ) );
                            if ( present_checksum != calculated_checksum ) {
                                    return protocol_error_record{ CHECKSUM_ERR, checksum_pos };
                            }
                            return std::get< 2 >( pack );
                    } );
        }
};

}  // namespace emlabcpp
