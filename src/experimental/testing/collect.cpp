#include "emlabcpp/experimental/testing/collect.h"

namespace emlabcpp::testing
{

collect_awaiter::collect_awaiter( collect_request req, collector& coll )
  : req( req )
  , col( coll )
{
}

void collect_awaiter::await_suspend( std::coroutine_handle< test_coroutine::promise_type > h )
{
        coro_handle       = h;
        h.promise().iface = this;
        col.exchange( req, [this]( const collect_server_client_group& var ) {
                match(
                    var,
                    [&]( const collect_reply& rpl ) {
                            res   = rpl.nid;
                            state = await_state::READY;
                    },
                    [&]( const tree_error_reply& err ) {
                            EMLABCPP_LOG( "Got an error: " << err );
                            state = await_state::ERRORED;
                    } );
        } );
}

collector::collector( collect_client_transmit_callback send_cb )
  : send_cb_( std::move( send_cb ) )
{
}

void collector::on_msg( const std::span< const uint8_t >& msg )
{
        using h = protocol::handler< collect_server_client_group >;
        h::extract( view_n( msg.data(), msg.size() ) )
            .match(
                [&]( const collect_server_client_group& req ) {
                        on_msg( req );
                },
                [&]( auto err ) {
                        EMLABCPP_LOG( "Failed to extract msg: " << err );
                } );
}
void collector::on_msg( const collect_server_client_group& var )
{
        reply_callback_( var );
}

collect_awaiter collector::set( node_id parent, std::string_view key, contiguous_container_type t )
{
        return collect_awaiter{
            collect_request{
                .parent        = parent,
                .expects_reply = true,
                .opt_key       = key_type_to_buffer( key ),
                .value         = t },
            *this };
}
collect_awaiter collector::append( node_id parent, contiguous_container_type t )
{
        return collect_awaiter{
            collect_request{
                .parent = parent, .expects_reply = true, .opt_key = std::nullopt, .value = t },
            *this };
}
void collector::set( node_id parent, std::string_view key, const value_type& val )
{
        send( collect_request{
            .parent        = parent,
            .expects_reply = false,
            .opt_key       = key_type_to_buffer( key ),
            .value         = val } );
}
void collector::append( node_id parent, const value_type& val )
{
        send( collect_request{
            .parent = parent, .expects_reply = false, .opt_key = std::nullopt, .value = val } );
}

void collector::exchange( const collect_request& req, collect_reply_callback cb )
{
        reply_callback_ = cb;
        send( req );
}

void collector::send( const collect_request& req )
{
        using h = protocol::handler< collect_request >;
        send_cb_( h::serialize( req ) );
}

collect_server::collect_server( pmr::memory_resource& mem_res, collect_server_transmit_callback send_cb )
  : tree_( mem_res )
  , send_cb_( std::move( send_cb ) )
{
}

void collect_server::on_msg( std::span< const uint8_t > data )
{
        using h = protocol::handler< collect_request >;
        h::extract( view_n( data.data(), data.size() ) )
            .match(
                [&]( const collect_request& req ) {
                        on_msg( req );
                },
                [&]( auto err ) {
                        EMLABCPP_LOG( "Failed to extract msg: " << err );
                } );
}

void collect_server::on_msg( const collect_request& req )
{
        // TODO: this may be a bad idea ...
        if ( tree_.empty() ) {
                if ( req.opt_key ) {
                        EMLABCPP_LOG( "collect tree is empty, making root object" );
                        tree_.make_object_node();
                } else {
                        EMLABCPP_LOG( "collect tree is empty, making root array" );
                        tree_.make_array_node();
                }
        }

        contiguous_request_adapter harn{ tree_ };

        either< node_id, contiguous_request_adapter_errors_enum > res =
            req.opt_key ? harn.insert( req.parent, *req.opt_key, req.value ) :
                          harn.insert( req.parent, req.value );
        res.match(
            [this, &req]( const node_id nid ) {
                    if ( !req.expects_reply ) {
                            return;
                    }
                    this->send( collect_reply{ nid } );
            },
            [this, &req]( const contiguous_request_adapter_errors_enum err ) {
                    this->send( tree_error_reply{ err, req.parent } );
            } );
}

void collect_server::send( const collect_server_client_group& val )
{
        using h = protocol::handler< collect_server_client_group >;
        send_cb_( h::serialize( val ) );
}

}  // namespace emlabcpp::testing
