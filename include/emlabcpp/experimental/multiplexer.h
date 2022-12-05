
#include "emlabcpp/protocol/tuple.h"

#pragma once

namespace protocol::protocol
{

using port_type = uint16_t;

template < std::size_t N >
using multiplexer = tuple< port_type, static_vector< uint8_t, N > >;

template < std::size_t N >
using multiplexer_value = typename multiplexer< N >::value_type;

template < std::size_t N >
using multiplexer_message = typename multiplexer< N >::message_type;

template < std::size_t N >
using multiplexer_handler = handler< multiplexer< N > >;

enum multiplexer_enum : uint8_t
{
        PORT_MATCH_ERROR = 0,
        PROTOCOL_ERROR   = 1,
};

static constexpr port_type multiplexer_service_id = 0;

using multiplexer_service_protocol = tuple< multiplexer_enum >;
using multiplexer_service_msg      = typename multiplexer_service_protocol::message_type;

template < std::size_t N, std::size_t M >
requires( M <= N )
    multiplexer_message< N > serialize_multiplexed( port_type port, const message< M >& m )
{
        return multiplexer_handler< N >::serialize( std::make_tuple( port, static_vector{ m } ) );
}

template < std::size_t N, typename BinaryCallable, typename MsgCallable >
bool extract_multiplexed(
    const multiplexer_message< N >& msg,
    BinaryCallable&&                handle_cb,
    MsgCallable&&                   msg_cb )
{
        return multiplexer_handler< N >::extract( msg )
            .convert_left( [&]( const auto& pack ) {
                    auto [id, msg] = pack;
                    bool success   = handle_cb( id, msg );
                    if ( !success ) {
                            multiplexer_service_msg msg;
                            msg[0] = PORT_MATCH_ERROR;
                            msg_cb( serialize_multiplexed( multiplexer_service_id, msg ) );
                    }
                    return success;
            } )
            .convert_right( [&]( const auto& ) {
                    multiplexer_service_msg msg;
                    msg[0] = PROTOCOL_ERROR;
                    msg_cb( serialize_multiplexed( multiplexer_service_id, msg ) );
                    return false;
            } )
            .join();
}

}  // namespace protocol::protocol
