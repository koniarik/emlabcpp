///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
/// and associated documentation files (the "Software"), to deal in the Software without
/// restriction, including without limitation the rights to use, copy, modify, merge, publish,
/// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or
/// substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
/// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
/// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#include "emlabcpp/experimental/testing/collect.h"

#include <utility>

namespace emlabcpp::testing
{

collect_awaiter::collect_awaiter( collect_request req, collector& coll )
  : req( std::move( req ) )
  , col( coll )
{
}

void collect_awaiter::await_suspend( const std::coroutine_handle< test_coroutine::promise_type > h )
{
        h.promise().iface = this;
        col.exchange( req, [this]( const collect_server_client_group& var ) {
                match(
                    var,
                    [this]( const collect_reply& rpl ) {
                            res   = rpl.nid;
                            state = coro::wait_state::READY;
                    },
                    [this]( const tree_error_reply& err ) {
                            std::ignore = err;
                            EMLABCPP_ERROR_LOG( "Got an error: ", err );
                            state = coro::wait_state::ERRORED;
                    } );
        } );
}

collector::collector( const protocol::channel_type chann, collect_client_transmit_callback send_cb )
  : channel_( chann )
  , send_cb_( std::move( send_cb ) )
{
}

void collector::on_msg( const std::span< const uint8_t >& msg )
{
        using h = protocol::handler< collect_server_client_group >;
        h::extract( view_n( msg.data(), msg.size() ) )
            .match(
                [this]( const collect_server_client_group& req ) {
                        on_msg( req );
                },
                []( const auto& err ) {
                        std::ignore = err;
                        EMLABCPP_ERROR_LOG( "Failed to extract msg: ", err );
                } );
}
void collector::on_msg( const collect_server_client_group& var )
{
        reply_callback_( var );
}

collect_awaiter
collector::set( const node_id parent, const std::string_view key, contiguous_container_type t )
{
        return collect_awaiter{
            collect_request{
                .parent        = parent,
                .expects_reply = true,
                .opt_key       = key_type_to_buffer( key ),
                .value         = t },
            *this };
}
collect_awaiter collector::append( const node_id parent, contiguous_container_type t )
{
        return collect_awaiter{
            collect_request{
                .parent = parent, .expects_reply = true, .opt_key = std::nullopt, .value = t },
            *this };
}
void collector::set( const node_id parent, const std::string_view key, const value_type& val )
{
        send( collect_request{
            .parent        = parent,
            .expects_reply = false,
            .opt_key       = key_type_to_buffer( key ),
            .value         = val } );
}
void collector::append( const node_id parent, const value_type& val )
{
        send( collect_request{
            .parent = parent, .expects_reply = false, .opt_key = std::nullopt, .value = val } );
}

void collector::exchange( const collect_request& req, collect_reply_callback cb )
{
        reply_callback_ = std::move( cb );
        send( req );
}

void collector::send( const collect_request& req )
{
        using h = protocol::handler< collect_request >;
        send_cb_( channel_, h::serialize( req ) );
}

collect_server::collect_server(
    const protocol::channel_type     chan,
    pmr::memory_resource&            mem_res,
    collect_server_transmit_callback send_cb )
  : channel_( chan )
  , tree_( mem_res )
  , send_cb_( std::move( send_cb ) )
{
}

void collect_server::on_msg( const std::span< const uint8_t > data )
{
        using h = protocol::handler< collect_request >;
        h::extract( view_n( data.data(), data.size() ) )
            .match(
                [this]( const collect_request& req ) {
                        on_msg( req );
                },
                []( const auto& err ) {
                        std::ignore = err;
                        EMLABCPP_ERROR_LOG( "Failed to extract msg: ", err );
                } );
}

void collect_server::on_msg( const collect_request& req )
{
        // TODO: this may be a bad idea ...
        if ( tree_.empty() ) {
                if ( req.opt_key ) {
                        EMLABCPP_DEBUG_LOG( "collect tree is empty, making root object" );
                        tree_.make_object_node();
                } else {
                        EMLABCPP_DEBUG_LOG( "collect tree is empty, making root array" );
                        tree_.make_array_node();
                }
        }

        contiguous_request_adapter harn{ tree_ };

        either< node_id, contiguous_request_adapter_errors > res =
            req.opt_key ? harn.insert( req.parent, *req.opt_key, req.value ) :
                          harn.insert( req.parent, req.value );
        res.match(
            [this, &req]( const node_id nid ) {
                    if ( !req.expects_reply ) {
                            return;
                    }
                    this->send( collect_reply{ nid } );
            },
            [this, &req]( const contiguous_request_adapter_errors err ) {
                    this->send( tree_error_reply{ err, req.parent } );
            } );
}

void collect_server::send( const collect_server_client_group& val )
{
        using h = protocol::handler< collect_server_client_group >;
        send_cb_( channel_, h::serialize( val ) );
}

}  // namespace emlabcpp::testing
