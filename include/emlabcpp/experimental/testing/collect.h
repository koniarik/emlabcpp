#include "emlabcpp/experimental/contiguous_tree/request_adapter.h"
#include "emlabcpp/experimental/function_view.h"
#include "emlabcpp/experimental/testing/convert.h"
#include "emlabcpp/experimental/testing/coroutine.h"
#include "emlabcpp/experimental/testing/protocol.h"
#include "emlabcpp/static_function.h"

#pragma once

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

using collect_reply_callback = static_function< void( const collect_server_client_group& ), 32 >;
using collect_client_transmit_callback =
    static_function< void( protocol::channel_type, const collect_client_server_message& ), 32 >;
using collect_server_transmit_callback =
    static_function< void( protocol::channel_type, const collect_server_client_message& ), 32 >;

class collector;

class [[nodiscard]] collect_awaiter : public test_awaiter_interface
{
public:
        collect_request                                       req;
        node_id                                               res{};
        await_state                                           state = await_state::WAITING;
        collector&                                            col;
        std::coroutine_handle< test_coroutine::promise_type > coro_handle;

        collect_awaiter( collect_request req, collector& coll );

        await_state get_state() const
        {
                return state;
        }

        [[nodiscard]] bool await_ready() const
        {
                return false;
        }

        void await_suspend( std::coroutine_handle< test_coroutine::promise_type > h );

        node_id await_resume()
        {
                coro_handle.promise().iface = nullptr;
                return res;
        }
};

// TODO: make concept for "testing module" -> has get_channel/on_msg...

class collector
{
public:
        collector( protocol::channel_type chann, collect_client_transmit_callback send_cb );

        collector( const collector& )            = delete;
        collector( collector&& )                 = delete;
        collector& operator=( const collector& ) = delete;
        collector& operator=( collector&& )      = delete;

        constexpr protocol::channel_type get_channel() const
        {
                return channel_;
        }

        void on_msg( const std::span< const uint8_t >& msg );
        void on_msg( const collect_server_client_group& var );

        collect_awaiter set( node_id parent, std::string_view key, contiguous_container_type t );
        collect_awaiter append( node_id parent, contiguous_container_type t );
        void            set( node_id parent, std::string_view key, const value_type& val );
        void            append( node_id parent, const value_type& val );

        template < typename Arg >
        void set( node_id parent, std::string_view key, const Arg& arg )
        {
                set( parent, key, value_type_converter< Arg >::to_value( arg ) );
        }
        template < typename Arg >
        void append( node_id parent, const Arg& arg )
        {
                append( parent, value_type_converter< Arg >::to_value( arg ) );
        }

        void exchange( const collect_request& req, collect_reply_callback cb );

private:
        void send( const collect_request& req );

        protocol::channel_type           channel_;
        collect_reply_callback           reply_callback_;
        collect_client_transmit_callback send_cb_;
};

class collect_server
{
public:
        collect_server(
            protocol::channel_type           chan,
            pmr::memory_resource&            mem_res,
            collect_server_transmit_callback send_cb );

        protocol::channel_type get_channel()
        {
                return channel_;
        }

        void on_msg( std::span< const uint8_t > data );
        void on_msg( const collect_request& req );

        void clear()
        {
                tree_.clear();
        }

        const data_tree& get_tree()
        {
                return tree_;
        }

private:
        void send( const collect_server_client_group& val );

        protocol::channel_type           channel_;
        data_tree                        tree_;
        collect_server_transmit_callback send_cb_;
};

}  // namespace emlabcpp::testing