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

#include "emlabcpp/experimental/contiguous_tree/request_adapter.h"
#include "emlabcpp/experimental/function_view.h"
#include "emlabcpp/experimental/testing/coroutine.h"
#include "emlabcpp/experimental/testing/protocol.h"
#include "emlabcpp/protocol/handler.h"
#include "emlabcpp/static_function.h"

#pragma once

namespace emlabcpp::testing
{

static constexpr protocol::channel_type params_channel = 3;

enum params_enum : uint8_t
{
        PARAM_VALUE       = 0x10,
        PARAM_CHILD       = 0x11,
        PARAM_CHILD_COUNT = 0x12,
        PARAM_KEY         = 0x13,
        PARAM_TYPE        = 0x14,
        PARAM_VALUE_KEY   = 0x15,
        PARAM_ERROR       = 0x16,
};

struct param_value_request
{
        static constexpr auto id = PARAM_VALUE;
        node_id               nid;
};

struct param_value_reply
{
        static constexpr auto id = PARAM_VALUE;
        value_type            value;
};

struct param_value_key_request
{
        static constexpr auto              id = PARAM_VALUE_KEY;
        node_id                            nid;
        std::variant< key_type, child_id > key;
};

struct param_value_key_reply
{
        static constexpr auto id = PARAM_VALUE_KEY;
        value_type            value;
};

struct param_child_request
{
        static constexpr auto              id = PARAM_CHILD;
        node_id                            parent;
        std::variant< key_type, child_id > chid;
};

struct param_child_reply
{
        static constexpr auto id = PARAM_CHILD;
        node_id               chid;
};

struct param_child_count_request
{
        static constexpr auto id = PARAM_CHILD_COUNT;
        node_id               parent;
};

struct param_child_count_reply
{
        static constexpr auto id = PARAM_CHILD_COUNT;
        child_count           count;
};

struct param_key_request
{
        static constexpr auto id = PARAM_KEY;
        node_id               nid;
        child_id              chid;
};

struct param_key_reply
{
        static constexpr auto id = PARAM_KEY;
        key_type              key;
};

struct param_type_request
{
        static constexpr auto id = PARAM_TYPE;
        node_id               nid;
};

struct param_type_reply
{
        static constexpr auto id = PARAM_TYPE;
        node_type_enum        type;
};

struct param_error
{
        static constexpr auto id = PARAM_ERROR;
        string_buffer         error;
};

using params_client_server_group = protocol::tag_group<
    param_value_request,
    param_child_request,
    param_child_count_request,
    param_key_request,
    param_type_request,
    param_value_key_request,
    param_error >;

using params_client_server_variant =
    typename protocol::traits_for< params_client_server_group >::value_type;
using params_client_server_message =
    typename protocol::handler< params_client_server_group >::message_type;

using params_server_client_group = protocol::tag_group<
    param_value_reply,
    param_child_reply,
    param_child_count_reply,
    param_key_reply,
    param_type_reply,
    param_value_key_reply,
    tree_error_reply >;

using params_server_client_variant =
    typename protocol::traits_for< params_server_client_group >::value_type;
using params_server_client_message =
    typename protocol::handler< params_server_client_group >::message_type;

using params_reply_callback = static_function< void( const params_server_client_variant& ), 32 >;
using params_client_transmit_callback =
    static_function< void( protocol::channel_type, const params_client_server_message& ), 32 >;
using params_server_transmit_callback =
    static_function< void( protocol::channel_type, const params_server_client_message& ), 32 >;

class parameters;

template < typename Processor >
struct [[nodiscard]] params_awaiter : public test_awaiter_interface
{
        Processor   proc;
        await_state state = await_state::WAITING;
        parameters& params;

        using request_type = decltype( proc.req );

        params_awaiter( request_type req, parameters& params )
          : proc{ .reply = {}, .req = req }
          , params( params )
        {
        }

        [[nodiscard]] await_state get_state() const override
        {
                return state;
        }

        [[nodiscard]] bool await_ready() const
        {
                return false;
        }

        void await_suspend( std::coroutine_handle< test_coroutine::promise_type > );

        decltype( auto ) await_resume()
        {
                return proc.reply;
        }
};

template < typename T >
struct param_value_processor
{
        T                   reply;
        param_value_request req;

        [[nodiscard]] bool set_value( const params_server_client_variant& var )
        {
                const auto* const val_ptr = std::get_if< param_value_reply >( &var );
                if ( val_ptr == nullptr ) {
                        return false;
                }
                auto opt_res = value_type_converter< T >::from_value( val_ptr->value );
                if ( opt_res ) {
                        reply = *opt_res;
                }
                return opt_res.has_value();
        }
};
template < typename T >
using param_value_awaiter = params_awaiter< param_value_processor< T > >;

template < typename T >
struct param_value_key_processor
{
        T                       reply;
        param_value_key_request req;

        [[nodiscard]] bool set_value( const params_server_client_variant& var )
        {
                const auto* const val_ptr = std::get_if< param_value_key_reply >( &var );
                if ( val_ptr == nullptr ) {
                        return false;
                }
                auto opt_res = value_type_converter< T >::from_value( val_ptr->value );
                if ( opt_res ) {
                        reply = *opt_res;
                }
                return opt_res.has_value();
        }
};
template < typename T >
using param_value_key_awaiter = params_awaiter< param_value_key_processor< T > >;

struct param_type_processor
{
        node_type_enum     reply;
        param_type_request req;
        using reply_type = param_type_reply;

        [[nodiscard]] bool set_value( const params_server_client_variant& var );
};
using param_type_awaiter = params_awaiter< param_type_processor >;
extern template class params_awaiter< param_type_processor >;

struct param_child_processor
{
        node_id             reply;
        param_child_request req;

        [[nodiscard]] bool set_value( const params_server_client_variant& var );
};
using param_child_awaiter = params_awaiter< param_child_processor >;
extern template class params_awaiter< param_child_processor >;

struct param_child_count_processor
{
        child_count               reply;
        param_child_count_request req;

        [[nodiscard]] bool set_value( const params_server_client_variant& var );
};
using param_child_count_awaiter = params_awaiter< param_child_count_processor >;
extern template class params_awaiter< param_child_count_processor >;

struct param_key_processor
{
        key_type          reply;
        param_key_request req;

        [[nodiscard]] bool set_value( const params_server_client_variant& var );
};
using param_key_awaiter = params_awaiter< param_key_processor >;
extern template class params_awaiter< param_key_processor >;

class parameters
{
public:
        parameters( protocol::channel_type chann, params_client_transmit_callback send_cb );

        [[nodiscard]] protocol::channel_type get_channel() const
        {
                return channel_;
        }

        void on_msg( std::span< const uint8_t > data );
        void on_msg( const params_server_client_variant& req );

        param_type_awaiter get_type( node_id nid );

        template < typename T >
        param_value_awaiter< T > get_value( const node_id node )
        {
                return param_value_awaiter< T >{ param_value_request{ node }, *this };
        }

        template < typename T >
        param_value_key_awaiter< T > get_value( const node_id node, const child_id chid )
        {
                return param_value_key_awaiter< T >{ param_value_key_request{ node, chid }, *this };
        }
        template < typename T >
        param_value_key_awaiter< T > get_value( const node_id node, const key_type& k )
        {
                return param_value_key_awaiter< T >{ param_value_key_request{ node, k }, *this };
        }
        template < typename T >
        param_value_key_awaiter< T > get_value( const node_id node, const std::string_view k )
        {
                return get_value< T >( node, key_type_to_buffer( k ) );
        }

        param_child_awaiter get_child( node_id nid, child_id chid );
        param_child_awaiter get_child( node_id nid, const key_type& key );
        param_child_awaiter get_child( node_id nid, std::string_view key );

        param_child_count_awaiter get_child_count( const node_id nid );

        param_key_awaiter get_key( node_id nid, child_id chid );

        void exchange( const params_client_server_variant& req, params_reply_callback reply_cb );

        void send( const params_client_server_variant& val );

private:
        protocol::channel_type          channel_;
        params_reply_callback           reply_cb_;
        params_client_transmit_callback send_cb_;
};

template < typename Processor >
void params_awaiter< Processor >::await_suspend(
    const std::coroutine_handle< test_coroutine::promise_type > h )
{
        h.promise().iface = this;
        params.exchange( proc.req, [this]( const params_server_client_variant& var ) {
                if ( !proc.set_value( var ) ) {
                        params.send( param_error{
                            string_to_buffer( "failed to process parameter value" ) } );
                        EMLABCPP_ERROR_LOG( "Setting value to processor errored" );
                        state = await_state::ERRORED;
                } else {
                        state = await_state::READY;
                }
        } );
}

class parameters_server
{
public:
        parameters_server(
            protocol::channel_type          chann,
            data_tree                       tree,
            params_server_transmit_callback send_cb );

        [[nodiscard]] protocol::channel_type get_channel() const
        {
                return channel_;
        }

        void on_msg( std::span< const uint8_t > data );
        void on_msg( const params_client_server_variant& req );

private:
        void on_req( const param_error& req );
        void on_req( const param_value_request& req );
        void on_req( const param_value_key_request& req );
        void on_req( const param_child_request& req );
        void on_req( const param_child_count_request& req );
        void on_req( const param_key_request& req );
        void on_req( const param_type_request& req );
        void reply_node_error( const contiguous_request_adapter_errors err, const node_id nid );
        void send( const params_server_client_variant& var );

        protocol::channel_type          channel_;
        data_tree                       tree_;
        params_server_transmit_callback send_cb_;
};
}  // namespace emlabcpp::testing
