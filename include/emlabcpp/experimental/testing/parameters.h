#include "emlabcpp/experimental/contiguous_tree/request_adapter.h"
#include "emlabcpp/experimental/function_view.h"
#include "emlabcpp/experimental/testing/coroutine.h"
#include "emlabcpp/experimental/testing/protocol.h"
#include "emlabcpp/static_function.h"

#pragma once

namespace emlabcpp::testing
{

enum params_enum : uint8_t
{
        PARAM_VALUE       = 0x10,
        PARAM_CHILD       = 0x11,
        PARAM_CHILD_COUNT = 0x12,
        PARAM_KEY         = 0x13,
        PARAM_TYPE        = 0x14,
        PARAM_VALUE_KEY   = 0x15,
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

using client_server_params_group = protocol::tag_group<
    param_value_request,
    param_child_request,
    param_child_count_request,
    param_key_request,
    param_type_request,
    param_value_key_request >;

using client_server_params_variant =
    typename protocol::traits_for< client_server_params_group >::value_type;

using server_client_params_group = protocol::tag_group<
    param_value_reply,
    param_child_reply,
    param_child_count_reply,
    param_key_reply,
    param_type_reply,
    param_value_key_reply,
    tree_error_reply >;

using server_client_params_variant =
    typename protocol::traits_for< server_client_params_group >::value_type;

using params_reply_callback = static_function< void( const server_client_params_variant& ), 32 >;
using params_send_callback  = function_view< void( std::span< const uint8_t > ) >;

class parameters;

template < typename Processor >
struct [[nodiscard]] params_awaiter : public test_awaiter_interface
{
        Processor                                             proc;
        await_state                                           state = await_state::WAITING;
        parameters&                                           params;
        std::coroutine_handle< test_coroutine::promise_type > coro_handle;

        using request_type = decltype( proc.req );

        params_awaiter( request_type req, parameters& params )
          : proc{ .reply = {}, .req = req }
          , params( params )
        {
        }

        await_state get_state() const
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
                // copy-pasta festival from collect
                coro_handle.promise().iface = nullptr;
                return proc.reply;
        }
};

template < typename T >
struct param_value_processor
{
        T                   reply;
        param_value_request req;

        [[nodiscard]] bool set_value( const server_client_params_variant& var )
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

        [[nodiscard]] bool set_value( const server_client_params_variant& var )
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

        [[nodiscard]] bool set_value( const server_client_params_variant& var )
        {
                const auto* const val_ptr = std::get_if< param_type_reply >( &var );
                if ( val_ptr == nullptr ) {
                        return false;
                }
                reply = val_ptr->type;
                return true;
        }
};
using param_type_awaiter = params_awaiter< param_type_processor >;

struct param_child_processor
{
        node_id             reply;
        param_child_request req;

        [[nodiscard]] bool set_value( const server_client_params_variant& var )
        {
                const auto* const val_ptr = std::get_if< param_child_reply >( &var );
                if ( val_ptr == nullptr ) {
                        return false;
                }
                reply = val_ptr->chid;
                return true;
        }
};
using param_child_awaiter = params_awaiter< param_child_processor >;

struct param_child_count_processor
{
        child_count               reply;
        param_child_count_request req;

        [[nodiscard]] bool set_value( const server_client_params_variant& var )
        {
                const auto* const val_ptr = std::get_if< param_child_count_reply >( &var );
                if ( val_ptr == nullptr ) {
                        return false;
                }
                reply = val_ptr->count;
                return true;
        }
};
using param_child_count_awaiter = params_awaiter< param_child_count_processor >;

struct param_key_processor
{
        key_type          reply;
        param_key_request req;

        [[nodiscard]] bool set_value( const server_client_params_variant& var )
        {
                const auto* const val_ptr = std::get_if< param_key_reply >( &var );
                if ( val_ptr == nullptr ) {
                        return false;
                }
                reply = val_ptr->key;
                return true;
        }
};
using param_key_awaiter = params_awaiter< param_key_processor >;

class parameters
{
public:
        parameters( params_send_callback send_cb )
          : send_cb_( std::move( send_cb ) )
        {
        }

        void on_msg( std::span< const uint8_t > data )
        {
                // TODO: this is copy pasta festival from collect...
                using h = protocol::handler< server_client_params_group >;
                h::extract( view_n( data.data(), data.size() ) )
                    .match(
                        [&]( const server_client_params_variant& req ) {
                                on_msg( req );
                        },
                        [&]( auto err ) {
                                EMLABCPP_LOG( "Failed to extract msg: " << err );
                        } );
        }

        void on_msg( const server_client_params_variant& req )
        {
                if ( !reply_cb_ ) {
                        return;
                }
                reply_cb_( req );
        }

        param_type_awaiter get_type( node_id nid )
        {
                return param_type_awaiter{ param_type_request{ nid }, *this };
        }

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

        param_child_awaiter get_child( node_id nid, child_id chid )
        {
                return param_child_awaiter{ param_child_request{ nid, chid }, *this };
        }
        param_child_awaiter get_child( node_id nid, const key_type& key )
        {
                return param_child_awaiter{ param_child_request{ nid, key }, *this };
        }

        param_child_awaiter get_child( node_id nid, std::string_view key )
        {
                return get_child( nid, key_type_to_buffer( key ) );
        }

        param_child_count_awaiter get_child_count( const node_id nid )
        {
                return param_child_count_awaiter{
                    param_child_count_request{ .parent = nid }, *this };
        }

        param_key_awaiter get_key( node_id nid, child_id chid )
        {
                return param_key_awaiter{ param_key_request{ nid, chid }, *this };
        }

        void exchange( const client_server_params_variant& req, params_reply_callback reply_cb )
        {
                reply_cb_ = reply_cb;
                send( req );
        }

        void send( const client_server_params_variant& val )
        {
                using h = protocol::handler< client_server_params_group >;
                send_cb_( h::serialize( val ) );
        }

private:
        params_reply_callback reply_cb_;
        params_send_callback  send_cb_;
};

template < typename Processor >
void params_awaiter< Processor >::await_suspend(
    std::coroutine_handle< test_coroutine::promise_type > h)
{
        coro_handle       = h;
        h.promise().iface = this;
        params.exchange( proc.req, [&]( const server_client_params_variant& var ) {
                if ( !proc.set_value( var ) ) {
                        EMLABCPP_LOG( "Setting value to processor errored" );
                        state = await_state::ERRORED;
                } else {
                        state = await_state::READY;
                }
        } );
}

class parameters_server
{
public:
        parameters_server( data_tree tree, params_send_callback send_cb )
          : tree_( tree )
          , send_cb_( std::move( send_cb ) )
        {
        }

        void on_msg( std::span< const uint8_t > data )
        {
                // TODO: this is copy pasta festival from collect...
                using h = protocol::handler< client_server_params_group >;
                h::extract( view_n( data.data(), data.size() ) )
                    .match(
                        [&]( const client_server_params_variant& req ) {
                                on_msg( req );
                        },
                        [&]( auto err ) {
                                EMLABCPP_LOG( "Failed to extract msg: " << err );
                        } );
        }

        void on_msg( const client_server_params_variant& req )
        {
                visit(
                    [&]( const auto& item ) {
                            on_req( item );
                    },
                    req );
        }

private:
        void on_req( const param_value_request& req )
        {
                const contiguous_request_adapter harn{ tree_ };

                harn.get_value( req.nid ).match(
                    [this]( const value_type& val ) {
                            send( param_value_reply{ val } );
                    },
                    [this, &req]( const contiguous_request_adapter_errors_enum err ) {
                            reply_node_error( err, req.nid );
                    } );
        }
        void on_req( const param_value_key_request& req )
        {
                const contiguous_request_adapter harn{ tree_ };

                harn.get_child( req.nid, req.key )
                    .bind_left( [this, &req, &harn]( const child_id chid ) {
                            return harn.get_value( chid );
                    } )
                    .match(
                        [this]( const value_type& val ) {
                                send( param_value_key_reply{ val } );
                        },
                        [this, &req]( const contiguous_request_adapter_errors_enum err ) {
                                reply_node_error( err, req.nid );
                        } );
        }
        void on_req( const param_child_request& req )
        {
                const contiguous_request_adapter harn{ tree_ };

                harn.get_child( req.parent, req.chid )
                    .match(
                        [this]( const node_id child ) {
                                send( param_child_reply{ child } );
                        },
                        [this, &req]( const contiguous_request_adapter_errors_enum err ) {
                                reply_node_error( err, req.parent );
                        } );
        }
        void on_req( const param_child_count_request& req )
        {
                const contiguous_request_adapter harn{ tree_ };

                harn.get_child_count( req.parent )
                    .match(
                        [this]( const child_id count ) {
                                send( param_child_count_reply{ count } );
                        },
                        [this, &req]( const contiguous_request_adapter_errors_enum err ) {
                                reply_node_error( err, req.parent );
                        } );
        }
        void on_req( const param_key_request& req )
        {
                const contiguous_request_adapter harn{ tree_ };

                harn.get_key( req.nid, req.chid )
                    .match(
                        [this]( const key_type& key ) {
                                send( param_key_reply{ key } );
                        },
                        [this, &req]( const contiguous_request_adapter_errors_enum err ) {
                                reply_node_error( err, req.nid );
                        } );
        }
        void on_req( const param_type_request& req )
        {
                const contiguous_request_adapter harn{ tree_ };

                harn.get_type( req.nid ).match(
                    [this]( const contiguous_tree_type_enum type ) {
                            send( param_type_reply{ type } );
                    },
                    [this, &req]( const contiguous_request_adapter_errors_enum err ) {
                            reply_node_error( err, req.nid );
                    } );
        }

        void reply_node_error( const contiguous_request_adapter_errors_enum err, const node_id nid )
        {
                if ( err == CONTIGUOUS_WRONG_TYPE ) {
                        EMLABCPP_LOG( "Failed to work with " << nid << ", wrong type of node" );
                } else if ( err == CONTIGUOUS_FULL ) {
                        EMLABCPP_LOG( "Failed to insert data, data storage is full" );
                } else if ( err == CONTIGUOUS_MISSING_NODE ) {
                        EMLABCPP_LOG( "Tree node " << nid << " is missing " );
                } else if ( err == CONTIGUOUS_CHILD_MISSING ) {
                        EMLABCPP_LOG( "Tree node child " << nid << " is missing " );
                }
                send( tree_error_reply{ .err = err, .nid = nid } );
        }

        void send( const server_client_params_variant& var )
        {
                using h = protocol::handler< server_client_params_group >;
                send_cb_( h::serialize( var ) );
        }

        data_tree            tree_;
        params_send_callback send_cb_;
};
}  // namespace emlabcpp::testing
