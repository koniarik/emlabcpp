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
                    msg.size() - Packet::prefix_decl::max_size - Packet::size_decl::max_size );

                protocol_serializer< size_type, endianess >::serialize_at(
                    std::span< uint8_t, size_size >{ msg.begin() + size_offset, size_size }, size );

                checksum_type chcksm =
                    Packet::get_checksum( view_n( msg.begin(), msg.size() - checksum_size ) );

                protocol_serializer< checksum_type, endianess >::serialize_at(
                    std::span< uint8_t, checksum_size >{ msg.end() - checksum_size, checksum_size },
                    chcksm );

                return msg;
        }

        static either< value_type, protocol_error_record > extract( const message_type& msg )
        {
                return sub_handler::extract( msg ).bind_left(
                    [&]( std::tuple< prefix_type, size_type, value_type, checksum_type > pack )
                        -> either< value_type, protocol_error_record > {
                            checksum_type present_checksum    = std::get< 3 >( pack );
                            checksum_type calculated_checksum = Packet::get_checksum(
                                view_n( msg.begin(), msg.size() - checksum_size ) );
                            if ( present_checksum != calculated_checksum ) {
                                    return protocol_error_record{};
                            }
                            return std::get< 2 >( pack );
                    } );
        }
};

}  // namespace emlabcpp
