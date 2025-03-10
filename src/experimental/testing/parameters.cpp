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

#include "emlabcpp/experimental/testing/parameters.h"

#include "emlabcpp/experimental/contiguous_tree/base.h"
#include "emlabcpp/experimental/contiguous_tree/request_adapter.h"
#include "emlabcpp/experimental/decompose.h"
#include "emlabcpp/experimental/multiplexer.h"
#include "emlabcpp/experimental/testing/base.h"
#include "emlabcpp/experimental/testing/protocol.h"
#include "emlabcpp/outcome.h"
#include "emlabcpp/protocol/handler.h"
#include "emlabcpp/result.h"
#include "emlabcpp/view.h"
#include "emlabcpp/visit.h"

#include <cstddef>
#include <span>
#include <string_view>
#include <tuple>
#include <utility>
#include <variant>

namespace emlabcpp::testing
{
bool param_type_processor::set_value( const params_server_client_variant& var )
{
        const auto* const val_ptr = std::get_if< param_type_reply >( &var );
        if ( val_ptr == nullptr )
                return false;
        reply = val_ptr->type;
        return true;
}
template struct params_awaiter< param_type_processor >;

[[nodiscard]] bool param_child_processor::set_value( const params_server_client_variant& var )
{
        const auto* const val_ptr = std::get_if< param_child_reply >( &var );
        if ( val_ptr == nullptr )
                return false;
        reply = val_ptr->chid;
        return true;
}
template struct params_awaiter< param_child_processor >;

[[nodiscard]] bool param_child_count_processor::set_value( const params_server_client_variant& var )
{
        const auto* const val_ptr = std::get_if< param_child_count_reply >( &var );
        if ( val_ptr == nullptr )
                return false;
        reply = val_ptr->count;
        return true;
}
template struct params_awaiter< param_child_count_processor >;

[[nodiscard]] bool param_key_processor::set_value( const params_server_client_variant& var )
{
        const auto* const val_ptr = std::get_if< param_key_reply >( &var );
        if ( val_ptr == nullptr )
                return false;
        reply = val_ptr->key;
        return true;
}
template struct params_awaiter< param_key_processor >;

parameters::parameters(
    const protocol::channel_type    chann,
    params_client_transmit_callback send_cb )
  : channel_( chann )
  , send_cb_( std::move( send_cb ) )
{
}

outcome parameters::on_msg( const std::span< const std::byte > data )
{
        // TODO: this is copy pasta festival from collect...
        using h = protocol::handler< params_server_client_group >;
        return h::extract( view_n( data.data(), data.size() ) )
            .match(
                [this]( const params_server_client_variant& req ) {
                        return on_msg( req );
                },
                []( const auto& err ) -> outcome {
                        std::ignore = err;
                        return ERROR;
                } );
}

outcome parameters::on_msg( const params_server_client_variant& req )
{
        if ( !reply_cb_ )
                return ERROR;
        reply_cb_( req );
        // TODO: maybe better error hndling can be done?
        return SUCCESS;
}

param_type_awaiter parameters::get_type( const node_id nid )
{
        return param_type_awaiter{ param_type_request{ nid }, *this };
}

param_child_awaiter parameters::get_child( const node_id nid, const child_id chid )
{
        return param_child_awaiter{ param_child_request{ nid, chid }, *this };
}

param_child_awaiter parameters::get_child( const node_id nid, const key_type& key )
{
        return param_child_awaiter{ param_child_request{ nid, key }, *this };
}

param_child_count_awaiter parameters::get_child_count( const node_id nid )
{
        return param_child_count_awaiter{ param_child_count_request{ .parent = nid }, *this };
}

param_key_awaiter parameters::get_key( const node_id nid, const child_id chid )
{
        return param_key_awaiter{ param_key_request{ nid, chid }, *this };
}

void parameters::exchange( const params_client_server_variant& req, params_reply_callback reply_cb )
{
        reply_cb_ = std::move( reply_cb );
        // TODO: the ignore is meh
        std::ignore = send( req );
}

result parameters::send( const params_client_server_variant& val )
{
        using h = protocol::handler< params_client_server_group >;
        return send_cb_( channel_, h::serialize( val ) );
}

parameters_server::parameters_server(
    const protocol::channel_type    chann,
    data_tree                       tree,
    params_server_transmit_callback send_cb )
  : channel_( chann )
  , tree_( std::move( tree ) )
  , send_cb_( std::move( send_cb ) )
{
}

outcome parameters_server::on_msg( const std::span< const std::byte > data )
{
        // TODO: this is copy pasta festival from collect...
        using h = protocol::handler< params_client_server_group >;
        return h::extract( view_n( data.data(), data.size() ) )
            .match(
                [this]( const params_client_server_variant& req ) {
                        return on_msg( req );
                },
                []( const auto& err ) -> outcome {
                        std::ignore = err;
                        return ERROR;
                } );
}

outcome parameters_server::on_msg( const params_client_server_variant& req )
{
        return visit(
            [this]( const auto& item ) {
                    return on_req( item );
            },
            req );
}

outcome parameters_server::on_req( const param_error& req ) const
{
        std::ignore = req;
        return FAILURE;
}

outcome parameters_server::on_req( const param_value_request& req )
{
        const contiguous_request_adapter harn{ tree_ };

        return harn.get_value( req.nid ).match(
            [this]( const value_type& val ) -> outcome {
                    return send( param_value_reply{ val } );
            },
            [this, &req]( const contiguous_request_adapter_errors err ) -> outcome {
                    return worst_of( reply_node_error( err, req.nid ), FAILURE );
            } );
}

outcome parameters_server::on_req( const param_value_key_request& req )
{
        const contiguous_request_adapter harn{ tree_ };

        return harn.get_child( req.nid, req.key )
            .bind_left( [&harn]( const child_id chid ) {
                    return harn.get_value( chid );
            } )
            .match(
                [this]( const value_type& val ) -> outcome {
                        return send( param_value_key_reply{ val } );
                },
                [this, &req]( const contiguous_request_adapter_errors err ) -> outcome {
                        return worst_of( reply_node_error( err, req.nid ), FAILURE );
                } );
}

outcome parameters_server::on_req( const param_child_request& req )
{
        const contiguous_request_adapter harn{ tree_ };

        return harn.get_child( req.parent, req.chid )
            .match(
                [this]( const node_id child ) -> outcome {
                        return send( param_child_reply{ child } );
                },
                [this, &req]( const contiguous_request_adapter_errors err ) -> outcome {
                        return worst_of( reply_node_error( err, req.parent ), FAILURE );
                } );
}

outcome parameters_server::on_req( const param_child_count_request& req )
{
        const contiguous_request_adapter harn{ tree_ };

        return harn.get_child_count( req.parent )
            .match(
                [this]( const child_id count ) -> outcome {
                        return send( param_child_count_reply{ count } );
                },
                [this, &req]( const contiguous_request_adapter_errors err ) -> outcome {
                        return worst_of( reply_node_error( err, req.parent ), FAILURE );
                } );
}

outcome parameters_server::on_req( const param_key_request& req )
{
        const contiguous_request_adapter harn{ tree_ };

        return harn.get_key( req.nid, req.chid )
            .match(
                [this]( const key_type& key ) -> outcome {
                        return send( param_key_reply{ key } );
                },
                [this, &req]( const contiguous_request_adapter_errors err ) -> outcome {
                        return worst_of( reply_node_error( err, req.nid ), FAILURE );
                } );
}

outcome parameters_server::on_req( const param_type_request& req )
{
        const contiguous_request_adapter harn{ tree_ };

        return harn.get_type( req.nid ).match(
            [this]( const contiguous_tree_type type ) -> outcome {
                    return send( param_type_reply{ type } );
            },
            [this, &req]( const contiguous_request_adapter_errors err ) -> outcome {
                    return worst_of( reply_node_error( err, req.nid ), FAILURE );
            } );
}

outcome parameters_server::reply_node_error(
    const contiguous_request_adapter_errors err,
    const node_id                           nid )
{
        return send( tree_error_reply{ .err = err, .nid = nid } );
}

result parameters_server::send( const params_server_client_variant& var )
{
        using h = protocol::handler< params_server_client_group >;
        return send_cb_( channel_, h::serialize( var ) );
}

};  // namespace emlabcpp::testing
