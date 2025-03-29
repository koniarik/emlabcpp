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

#include "../../static_function.h"
#include "../contiguous_tree/request_adapter.h"
#include "../function_view.h"
#include "./convert.h"
#include "./coroutine.h"
#include "./protocol.h"

#include <source_location>

namespace emlabcpp::testing
{

static constexpr protocol::channel_type collect_channel = 2;

using collect_key_type   = key_type_buffer;
using collect_value_type = std::variant< value_type, contiguous_container_type >;

struct collect_request
{
        node_id                           parent;
        bool                              expects_reply;
        std::optional< collect_key_type > opt_key;
        collect_value_type                value;
};

struct collect_reply
{
        node_id nid;
};

using collect_server_client_group = std::variant< collect_reply, tree_error_reply >;
using collect_server_client_message =
    typename protocol::handler< collect_server_client_group >::message_type;
using collect_client_server_message = typename protocol::handler< collect_request >::message_type;

using collect_reply_callback = static_function< void( collect_server_client_group const& ), 32 >;
using collect_client_transmit_callback =
    static_function< result( protocol::channel_type, collect_client_server_message const& ), 32 >;
using collect_server_transmit_callback =
    static_function< result( protocol::channel_type, collect_server_client_message const& ), 32 >;

class collector;

class [[nodiscard]] collect_awaiter : public wait_interface
{
public:
        collect_request req;
        node_id         res{};
        coro_state      state = coro_state::WAITING;
        collector&      col;

        collect_awaiter( collect_request const& req, collector& coll );

        [[nodiscard]] coro_state get_state() const override
        {
                return state;
        }

        void tick() override
        {
        }

        [[nodiscard]] bool await_ready() const
        {
                return false;
        }

        void await_suspend( std::coroutine_handle< coroutine< void >::promise_type > h );

        [[nodiscard]] node_id await_resume() const
        {
                return res;
        }
};

// TODO: make concept for "testing module" -> has get_channel/on_msg...

class collector
{
public:
        collector( protocol::channel_type chann, collect_client_transmit_callback send_cb );

        collector( collector const& )            = delete;
        collector( collector&& )                 = delete;
        collector& operator=( collector const& ) = delete;
        collector& operator=( collector&& )      = delete;

        [[nodiscard]] constexpr protocol::channel_type get_channel() const
        {
                return channel_;
        }

        outcome on_msg( std::span< std::byte const > const& msg );
        outcome on_msg( collect_server_client_group const& var );

        collect_awaiter
             set( node_id const parent, std::string_view key, contiguous_container_type t );
        bool set( node_id const parent, std::string_view key, value_type const& val );

        collect_awaiter set( std::string_view key, contiguous_container_type t )
        {
                return set( 0, key, t );
        }

        bool set( std::string_view key, value_type const& val )
        {
                return set( 0, key, val );
        }

        template < typename Arg >
        bool set( node_id parent, std::string_view key, Arg const& arg )
        {
                return set( parent, key, value_type_converter< Arg >::to_value( arg ) );
        }

        template < typename Arg >
        bool set( std::string_view key, Arg const& arg )
        {
                return set( 0, key, value_type_converter< Arg >::to_value( arg ) );
        }

        collect_awaiter append( node_id const parent, contiguous_container_type t );
        bool            append( node_id const parent, value_type const& val );

        template < typename Arg >
        bool append( node_id parent, Arg const& arg )
        {
                return append( parent, value_type_converter< Arg >::to_value( arg ) );
        }

        bool exchange( collect_request const& req, collect_reply_callback cb );

private:
        result send( collect_request const& req );

        protocol::channel_type           channel_;
        collect_reply_callback           reply_callback_;
        collect_client_transmit_callback send_cb_;
};

inline status_awaiter
expect( collector& c, bool expr, std::source_location loc = std::source_location::current() )
{
        if ( !expr ) {
                c.set( "sfile", loc.file_name() );
                c.set( "sfun", loc.function_name() );
                c.set( "sline", loc.line() );
        }
        return expect( expr );
}

class collect_server
{
public:
        collect_server(
            protocol::channel_type           chan,
            pmr::memory_resource&            mem_res,
            collect_server_transmit_callback send_cb );

        [[nodiscard]] protocol::channel_type get_channel() const
        {
                return channel_;
        }

        outcome on_msg( std::span< std::byte const > data );
        outcome on_msg( collect_request const& req );

        void clear()
        {
                tree_.clear();
        }

        [[nodiscard]] data_tree const& get_tree() const
        {
                return tree_;
        }

private:
        result send( collect_server_client_group const& val );

        protocol::channel_type           channel_;
        data_tree                        tree_;
        collect_server_transmit_callback send_cb_;
};

}  // namespace emlabcpp::testing
