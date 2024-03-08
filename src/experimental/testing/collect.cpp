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

#include "emlabcpp/either.h"
#include "emlabcpp/experimental/contiguous_tree/base.h"
#include "emlabcpp/experimental/contiguous_tree/request_adapter.h"
#include "emlabcpp/experimental/coro/recursive.h"
#include "emlabcpp/experimental/decompose.h"
#include "emlabcpp/experimental/logging.h"
#include "emlabcpp/experimental/multiplexer.h"
#include "emlabcpp/experimental/testing/base.h"
#include "emlabcpp/experimental/testing/coroutine.h"
#include "emlabcpp/experimental/testing/protocol.h"
#include "emlabcpp/match.h"
#include "emlabcpp/outcome.h"
#include "emlabcpp/pmr/memory_resource.h"
#include "emlabcpp/protocol/handler.h"
#include "emlabcpp/result.h"
#include "emlabcpp/view.h"

#include <coroutine>
#include <cstddef>
#include <optional>
#include <span>
#include <string_view>
#include <tuple>
#include <utility>

namespace emlabcpp::testing
{

collect_awaiter::collect_awaiter( const collect_request& req, collector& coll )
  : req( req )
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
                            EMLABCPP_ERROR_LOG( "Got an error: ", decompose( err ) );
                            state = coro::wait_state::ERRORED;
                    } );
        } );
}

collector::collector( const protocol::channel_type chann, collect_client_transmit_callback send_cb )
  : channel_( chann )
  , send_cb_( std::move( send_cb ) )
{
}

outcome collector::on_msg( const std::span< const std::byte >& msg )
{
        using h = protocol::handler< collect_server_client_group >;
        return h::extract( view_n( msg.data(), msg.size() ) )
            .match(
                [this]( const collect_server_client_group& req ) {
                        return on_msg( req );
                },
                []( const auto& err ) -> outcome {
                        std::ignore = err;
                        EMLABCPP_ERROR_LOG( "Failed to extract msg: ", err );
                        return ERROR;
                } );
}

outcome collector::on_msg( const collect_server_client_group& var )
{
        if ( reply_callback_ ) {
                reply_callback_( var );
                return SUCCESS;
        } else {
                return ERROR;
        }
}

collect_awaiter
collector::set( const node_id parent, const std::string_view key, contiguous_container_type t )
{
        return collect_awaiter{
            collect_request{
                .parent = parent, .expects_reply = true, .opt_key = key_type( key ), .value = t },
            *this };
}

collect_awaiter collector::append( const node_id parent, contiguous_container_type t )
{
        return collect_awaiter{
            collect_request{
                .parent = parent, .expects_reply = true, .opt_key = std::nullopt, .value = t },
            *this };
}

bool collector::set( const node_id parent, const std::string_view key, const value_type& val )
{
        EMLABCPP_DEBUG_LOG(
            "Sending collect request for parent ", parent, " key: ", key, " value: ", val );
        return send( collect_request{
                   .parent        = parent,
                   .expects_reply = false,
                   .opt_key       = key_type( key ),
                   .value         = val } ) == SUCCESS;
}

bool collector::append( const node_id parent, const value_type& val )
{
        return send( collect_request{
                   .parent        = parent,
                   .expects_reply = false,
                   .opt_key       = std::nullopt,
                   .value         = val } ) == SUCCESS;
}

bool collector::exchange( const collect_request& req, collect_reply_callback cb )
{
        reply_callback_ = std::move( cb );
        return send( req ) == SUCCESS;
}

result collector::send( const collect_request& req )
{
        using h  = protocol::handler< collect_request >;
        auto msg = h::serialize( req );
        return send_cb_( channel_, msg );
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

outcome collect_server::on_msg( const std::span< const std::byte > data )
{
        using h = protocol::handler< collect_request >;
        EMLABCPP_DEBUG_LOG( "got msg: ", collect_client_server_message{ data } );
        return h::extract( view_n( data.data(), data.size() ) )
            .match(
                [this]( const collect_request& req ) {
                        return on_msg( req );
                },
                []( const auto& err ) -> outcome {
                        std::ignore = err;
                        EMLABCPP_ERROR_LOG( "Failed to extract msg: ", err );
                        return ERROR;
                } );
}

outcome collect_server::on_msg( const collect_request& req )
{
        EMLABCPP_DEBUG_LOG(
            "got collect request: parent:",
            req.parent,
            " reply:",
            req.expects_reply,
            " opt_key:",
            req.opt_key,
            " value:",
            req.value );
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
        return res.match(
            [this, &req]( const node_id nid ) -> result {
                    if ( !req.expects_reply )
                            return SUCCESS;
                    return this->send( collect_reply{ nid } );
            },
            [this, &req]( const contiguous_request_adapter_errors err ) {
                    return this->send( tree_error_reply{ err, req.parent } );
            } );
}

result collect_server::send( const collect_server_client_group& val )
{
        using h = protocol::handler< collect_server_client_group >;
        return send_cb_( channel_, h::serialize( val ) );
}

}  // namespace emlabcpp::testing
