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

#include "../match.h"
#include "./handler.h"
#include "./packet.h"
#include "./streams.h"

namespace emlabcpp::protocol
{

template < typename Packet >
struct packet_handler
{
        using message_type  = typename Packet::message_type;
        using prefix_type   = typename Packet::prefix_type;
        using payload_type  = typename Packet::payload_type;
        using value_type    = typename Packet::value_type;
        using size_type     = typename Packet::size_type;
        using checksum_type = typename Packet::checksum_type;

        using sub_handler                          = handler< Packet >;
        static constexpr std::size_t size_offset   = Packet::prefix_traits::max_size;
        static constexpr std::size_t checksum_size = Packet::checksum_traits::max_size;
        static constexpr std::size_t size_size     = Packet::size_traits::max_size;
        static constexpr auto        endianess     = Packet::endianess;

        static message_type serialize( value_type const& val )
        {

                message_type msg = sub_handler::serialize(
                    std::make_tuple( Packet::prefix, val, checksum_type{} ) );

                checksum_type const chcksm =
                    Packet::get_checksum( view_n( msg.begin(), msg.size() - checksum_size ) );

                serializer< checksum_type, endianess >::serialize_at(
                    std::span< std::byte, checksum_size >{
                        msg.end() - checksum_size, checksum_size },
                    chcksm );

                return msg;
        }

        static std::variant< value_type, error_record >
        extract( view< std::byte const* > const& msg )
        {
                return match(
                    sub_handler::extract( msg ),
                    [&msg]( std::tuple< prefix_type, value_type, checksum_type > pack )
                        -> std::variant< value_type, error_record > {
                            checksum_type const present_checksum = std::get< 2 >( pack );
                            std::size_t const   checksum_pos     = msg.size() - checksum_size;
                            view< std::byte const* > const area =
                                view_n( msg.begin(), checksum_pos );
                            checksum_type const calculated_checksum = Packet::get_checksum( area );

                            if ( present_checksum != calculated_checksum )
                                    return error_record{
                                        .error_mark = CHECKSUM_ERR, .offset = checksum_pos };

                            return std::get< 1 >( pack );
                    },
                    [&]( error_record const err ) -> std::variant< value_type, error_record > {
                            return err;
                    } );
        }
};

}  // namespace emlabcpp::protocol
