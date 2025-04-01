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

#include "emlabcpp/experimental/testing/collect.h"

#include "emlabcpp/experimental/contiguous_tree/base.h"
#include "emlabcpp/experimental/contiguous_tree/request_adapter.h"
#include "emlabcpp/experimental/coro/recursive.h"
#include "emlabcpp/experimental/decompose.h"
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

collect_awaiter::collect_awaiter( collect_request const& req, collector& coll )
  : req( req )
  , col( coll )
{
}

void collect_awaiter::await_suspend(
    std::coroutine_handle< coroutine< void >::promise_type > const h )
{
        h.promise().iface = this;
        col.exchange( req, [this]( collect_server_client_group const& var ) {
                match(
                    var,
                    [this]( collect_reply const& rpl ) {
                            res   = rpl.nid;
                            state = coro_state::DONE;
                    },
                    [this]( tree_error_reply const& err ) {
                            std::ignore = err;
                            state       = coro_state::ERRORED;
                    } );
        } );
}

collector::collector( protocol::channel_type const chann, collect_client_transmit_callback send_cb )
  : channel_( chann )
  , send_cb_( std::move( send_cb ) )
{
}

outcome collector::on_msg( std::span< std::byte const > const& msg )
{
        using h = protocol::handler< collect_server_client_group >;
        return match(
            h::extract( view_n( msg.data(), msg.size() ) ),
            [this]( collect_server_client_group const& req ) {
                    return on_msg( req );
            },
            []( auto const& err ) -> outcome {
                    std::ignore = err;
                    return outcome::ERROR;
            } );
}

outcome collector::on_msg( collect_server_client_group const& var )
{
        if ( reply_callback_ ) {
                reply_callback_( var );
                return outcome::SUCCESS;
        } else {
                return outcome::ERROR;
        }
}

collect_awaiter
collector::set( node_id const parent, std::string_view const key, contiguous_container_type t )
{
        return collect_awaiter{
            collect_request{
                .parent = parent, .expects_reply = true, .opt_key = key_type( key ), .value = t },
            *this };
}

collect_awaiter collector::append( node_id const parent, contiguous_container_type t )
{
        return collect_awaiter{
            collect_request{
                .parent = parent, .expects_reply = true, .opt_key = std::nullopt, .value = t },
            *this };
}

bool collector::set( node_id const parent, std::string_view const key, value_type const& val )
{
        return send( collect_request{
                   .parent        = parent,
                   .expects_reply = false,
                   .opt_key       = key_type( key ),
                   .value         = val } ) == result::SUCCESS;
}

bool collector::append( node_id const parent, value_type const& val )
{
        return send( collect_request{
                   .parent        = parent,
                   .expects_reply = false,
                   .opt_key       = std::nullopt,
                   .value         = val } ) == result::SUCCESS;
}

bool collector::exchange( collect_request const& req, collect_reply_callback cb )
{
        reply_callback_ = std::move( cb );
        return send( req ) == result::SUCCESS;
}

result collector::send( collect_request const& req )
{
        using h  = protocol::handler< collect_request >;
        auto msg = h::serialize( req );
        return send_cb_( channel_, msg );
}

collect_server::collect_server(
    protocol::channel_type const     chan,
    pmr::memory_resource&            mem_res,
    collect_server_transmit_callback send_cb )
  : channel_( chan )
  , tree_( mem_res )
  , send_cb_( std::move( send_cb ) )
{
}

outcome collect_server::on_msg( std::span< std::byte const > const data )
{
        using h = protocol::handler< collect_request >;
        return match(
            h::extract( view_n( data.data(), data.size() ) ),
            [this]( collect_request const& req ) {
                    return on_msg( req );
            },
            []( auto const& err ) -> outcome {
                    std::ignore = err;
                    return result::ERROR;
            } );
}

outcome collect_server::on_msg( collect_request const& req )
{
        // TODO: this may be a bad idea ...
        if ( tree_.empty() ) {
                if ( req.opt_key )
                        tree_.make_object_node();
                else
                        tree_.make_array_node();
        }

        contiguous_request_adapter harn{ tree_ };

        std::variant< node_id, contiguous_request_adapter_errors > res =
            req.opt_key ? harn.insert( req.parent, *req.opt_key, req.value ) :
                          harn.insert( req.parent, req.value );
        return match(
            res,
            [this, &req]( node_id const nid ) -> result {
                    if ( !req.expects_reply )
                            return result::SUCCESS;
                    return this->send( collect_reply{ nid } );
            },
            [this, &req]( contiguous_request_adapter_errors const err ) {
                    return this->send( tree_error_reply{ .err = err, .nid = req.parent } );
            } );
}

result collect_server::send( collect_server_client_group const& val )
{
        using h = protocol::handler< collect_server_client_group >;
        return send_cb_( channel_, h::serialize( val ) );
}

}  // namespace emlabcpp::testing
