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

#include "emlabcpp/outcome.h"
#include "emlabcpp/protocol/endpoint.h"
#include "emlabcpp/protocol/handler.h"
#include "emlabcpp/protocol/tuple.h"
#include "emlabcpp/static_vector.h"

namespace emlabcpp::protocol
{

using channel_type = uint16_t;

using multiplexer_channel_handler = handler< channel_type, std::endian::little >;

template < std::size_t N >
using multiplexer_payload = tuple< std::endian::little, channel_type, sizeless_message< N > >;

template < std::size_t N >
using multiplexer_value = typename multiplexer_payload< N >::value_type;

template < std::size_t N >
using multiplexer_message = typename multiplexer_payload< N >::message_type;

template < std::size_t N >
using multiplexer_handler = handler< multiplexer_payload< N > >;

enum multiplexer_enum : uint8_t
{
        PORT_MATCH_ERROR = 0,
        PROTOCOL_ERROR   = 1,
};

static constexpr channel_type multiplexer_service_id = 0;

using multiplexer_service_protocol = std::tuple< multiplexer_enum >;
using multiplexer_service_msg      = typename handler< multiplexer_service_protocol >::message_type;

inline std::tuple< bool, std::span< std::byte > > serialize_multiplexed(
    channel_type                 channel,
    std::span< const std::byte > data,
    std::span< std::byte >       target )
{
        using chan_ser = protocol::serializer< channel_type, std::endian::little >;
        if ( target.size() < chan_ser::max_size + data.size() )
                return { false, {} };

        chan_ser::serialize_at( target.subspan< 0, chan_ser::max_size >(), channel );
        std::memcpy( target.data() + chan_ser::max_size, data.data(), data.size() );

        return { false, target.subspan( 0, chan_ser::max_size + data.size() ) };
}

template < std::size_t N >
multiplexer_message< N > serialize_multiplexed( channel_type channel, const message< N >& m )
{
        return multiplexer_handler< N >::serialize(
            std::make_tuple( channel, sizeless_message< N >( m ) ) );
}

template < typename BinaryCallable >
outcome extract_multiplexed( const std::span< const std::byte >& msg, BinaryCallable handle_cb )
{
        using chan_ser = protocol::serializer< channel_type, std::endian::little >;
        if ( msg.size() < chan_ser::max_size )
                return ERROR;
        channel_type id = chan_ser::deserialize( msg.subspan< 0, chan_ser::max_size >() );

        // TODO: there might be a better way than callback?
        return handle_cb( id, msg.subspan( chan_ser::max_size ) );
}

template < typename T >
concept slot = requires( T s, std::span< const std::byte > data ) {
        { s.get_channel() } -> std::convertible_to< channel_type >;
        { s.on_msg( data ) } -> std::convertible_to< outcome >;
};

template < slot... Slotted >
outcome multiplexed_dispatch( channel_type chann, const auto& data, Slotted&... slotted )
{
        outcome res = SUCCESS;

        // TODO: assert that channels are unique
        auto f = [&]< typename T >( T& item ) {
                if ( chann != item.get_channel() )
                        return false;
                res = item.on_msg( data );
                return true;
        };
        if ( !( f( slotted ) || ... || false ) )
                res = ERROR;
        return res;
}

template < typename Packet >
class multiplexed_endpoint
{
public:
        using message_type    = typename Packet::message_type;
        using payload_message = typename Packet::payload_type::template nth_def< 1 >;

        // TODO: !!! use "has_static_size" concept!
        // TODO: maybe message_like concept?
        template < std::size_t N >
        message_type serialize( channel_type chann, const message< N >& msg )
        {
                return ep.serialize( std::make_tuple( chann, payload_message{ msg } ) );
        }

        template < std::size_t N >
        message_type serialize( channel_type chann, const sizeless_message< N >& msg )
        {
                return ep.serialize( std::make_tuple( chann, payload_message{ msg } ) );
        }

        template < typename Container >
        void insert( Container&& data )
        {
                ep.insert( std::forward< Container >( data ) );
        }

        std::variant< std::size_t, std::tuple< channel_type, payload_message >, error_record >
        get_value()
        {
                return ep.get_value();
        }

        template < typename NextCallable, typename ValueCallable, typename ErrorCallable >
        auto match_value( NextCallable&& nc, ValueCallable&& vc, ErrorCallable&& ec )
        {
                return match(
                    ep.get_value(),
                    std::forward< NextCallable >( nc ),
                    [&vc]( const std::tuple< channel_type, payload_message >& payload ) {
                            return std::apply( std::forward< ValueCallable >( vc ), payload );
                    },
                    std::forward< ErrorCallable >( ec ) );
        }

        template < typename... Slotted >
        outcome dispatch_value( Slotted&... slotted )
        {
                return match(
                    ep.get_value(),
                    []( const std::size_t ) -> outcome {
                            return SUCCESS;
                    },
                    [&slotted...]( const std::tuple< channel_type, payload_message >& payload ) {
                            const auto& [id, data] = payload;
                            return multiplexed_dispatch( id, data, slotted... );
                    },
                    []( const error_record& ) -> outcome {
                            return ERROR;
                    } );
        }

private:
        endpoint< Packet, Packet > ep;
};

}  // namespace emlabcpp::protocol
