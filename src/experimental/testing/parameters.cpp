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
bool param_type_processor::set_value( params_server_client_variant const& var )
{
        auto const* const val_ptr = std::get_if< param_type_reply >( &var );
        if ( val_ptr == nullptr )
                return false;
        reply = val_ptr->type;
        return true;
}
template struct params_awaiter< param_type_processor >;

[[nodiscard]] bool param_child_processor::set_value( params_server_client_variant const& var )
{
        auto const* const val_ptr = std::get_if< param_child_reply >( &var );
        if ( val_ptr == nullptr )
                return false;
        reply = val_ptr->chid;
        return true;
}
template struct params_awaiter< param_child_processor >;

[[nodiscard]] bool param_child_count_processor::set_value( params_server_client_variant const& var )
{
        auto const* const val_ptr = std::get_if< param_child_count_reply >( &var );
        if ( val_ptr == nullptr )
                return false;
        reply = val_ptr->count;
        return true;
}
template struct params_awaiter< param_child_count_processor >;

[[nodiscard]] bool param_key_processor::set_value( params_server_client_variant const& var )
{
        auto const* const val_ptr = std::get_if< param_key_reply >( &var );
        if ( val_ptr == nullptr )
                return false;
        reply = val_ptr->key;
        return true;
}
template struct params_awaiter< param_key_processor >;

parameters::parameters(
    protocol::channel_type const    chann,
    params_client_transmit_callback send_cb )
  : channel_( chann )
  , send_cb_( std::move( send_cb ) )
{
}

outcome parameters::on_msg( std::span< std::byte const > const data )
{
        // TODO: this is copy pasta festival from collect...
        using h = protocol::handler< params_server_client_group >;
        return match(
            h::extract( view_n( data.data(), data.size() ) ),
            [this]( params_server_client_variant const& req ) {
                    return on_msg( req );
            },
            []( auto const& err ) -> outcome {
                    std::ignore = err;
                    return ERROR;
            } );
}

outcome parameters::on_msg( params_server_client_variant const& req )
{
        if ( !reply_cb_ )
                return ERROR;
        reply_cb_( req );
        // TODO: maybe better error hndling can be done?
        return SUCCESS;
}

param_type_awaiter parameters::get_type( node_id const nid )
{
        return param_type_awaiter{ param_type_request{ nid }, *this };
}

param_child_awaiter parameters::get_child( node_id const nid, child_id const chid )
{
        return param_child_awaiter{ param_child_request{ nid, chid }, *this };
}

param_child_awaiter parameters::get_child( node_id const nid, key_type const& key )
{
        return param_child_awaiter{ param_child_request{ nid, key }, *this };
}

param_child_count_awaiter parameters::get_child_count( node_id const nid )
{
        return param_child_count_awaiter{ param_child_count_request{ .parent = nid }, *this };
}

param_key_awaiter parameters::get_key( node_id const nid, child_id const chid )
{
        return param_key_awaiter{ param_key_request{ nid, chid }, *this };
}

void parameters::exchange( params_client_server_variant const& req, params_reply_callback reply_cb )
{
        reply_cb_ = std::move( reply_cb );
        // TODO: the ignore is meh
        std::ignore = send( req );
}

result parameters::send( params_client_server_variant const& val )
{
        using h = protocol::handler< params_client_server_group >;
        return send_cb_( channel_, h::serialize( val ) );
}

parameters_server::parameters_server(
    protocol::channel_type const    chann,
    data_tree                       tree,
    params_server_transmit_callback send_cb )
  : channel_( chann )
  , tree_( std::move( tree ) )
  , send_cb_( std::move( send_cb ) )
{
}

outcome parameters_server::on_msg( std::span< std::byte const > const data )
{
        // TODO: this is copy pasta festival from collect...
        using h = protocol::handler< params_client_server_group >;
        return match(
            h::extract( view_n( data.data(), data.size() ) ),
            [this]( params_client_server_variant const& req ) {
                    return on_msg( req );
            },
            []( auto const& err ) -> outcome {
                    std::ignore = err;
                    return ERROR;
            } );
}

outcome parameters_server::on_msg( params_client_server_variant const& req )
{
        return visit(
            [this]( auto const& item ) {
                    return on_req( item );
            },
            req );
}

outcome parameters_server::on_req( param_error const& req ) const
{
        std::ignore = req;
        return FAILURE;
}

outcome parameters_server::on_req( param_value_request const& req )
{
        contiguous_request_adapter const harn{ tree_ };

        return match(
            harn.get_value( req.nid ),
            [this]( value_type const& val ) -> outcome {
                    return send( param_value_reply{ val } );
            },
            [this, &req]( contiguous_request_adapter_errors const err ) -> outcome {
                    return worst_of( reply_node_error( err, req.nid ), FAILURE );
            } );
}

outcome parameters_server::on_req( param_value_key_request const& req )
{
        contiguous_request_adapter const harn{ tree_ };

        auto return_err = [&req, this]( contiguous_request_adapter_errors const err ) -> outcome {
                return worst_of( reply_node_error( err, req.nid ), FAILURE );
        };

        return match(
            harn.get_child( req.nid, req.key ),
            [this, &harn, &return_err]( child_id const chid ) -> outcome {
                    return match(
                        harn.get_value( chid ),
                        [this]( value_type const& val ) -> outcome {
                                return send( param_value_key_reply{ val } );
                        },
                        return_err );
            },
            return_err );
}

outcome parameters_server::on_req( param_child_request const& req )
{
        contiguous_request_adapter const harn{ tree_ };

        return match(
            harn.get_child( req.parent, req.chid ),
            [this]( node_id const child ) -> outcome {
                    return send( param_child_reply{ child } );
            },
            [this, &req]( contiguous_request_adapter_errors const err ) -> outcome {
                    return worst_of( reply_node_error( err, req.parent ), FAILURE );
            } );
}

outcome parameters_server::on_req( param_child_count_request const& req )
{
        contiguous_request_adapter const harn{ tree_ };

        return match(
            harn.get_child_count( req.parent ),
            [this]( child_id const count ) -> outcome {
                    return send( param_child_count_reply{ count } );
            },
            [this, &req]( contiguous_request_adapter_errors const err ) -> outcome {
                    return worst_of( reply_node_error( err, req.parent ), FAILURE );
            } );
}

outcome parameters_server::on_req( param_key_request const& req )
{
        contiguous_request_adapter const harn{ tree_ };

        return match(
            harn.get_key( req.nid, req.chid ),
            [this]( key_type const& key ) -> outcome {
                    return send( param_key_reply{ key } );
            },
            [this, &req]( contiguous_request_adapter_errors const err ) -> outcome {
                    return worst_of( reply_node_error( err, req.nid ), FAILURE );
            } );
}

outcome parameters_server::on_req( param_type_request const& req )
{
        contiguous_request_adapter const harn{ tree_ };

        return match(
            harn.get_type( req.nid ),
            [this]( contiguous_tree_type const type ) -> outcome {
                    return send( param_type_reply{ type } );
            },
            [this, &req]( contiguous_request_adapter_errors const err ) -> outcome {
                    return worst_of( reply_node_error( err, req.nid ), FAILURE );
            } );
}

outcome parameters_server::reply_node_error(
    contiguous_request_adapter_errors const err,
    node_id const                           nid )
{
        return send( tree_error_reply{ .err = err, .nid = nid } );
}

result parameters_server::send( params_server_client_variant const& var )
{
        using h = protocol::handler< params_server_client_group >;
        return send_cb_( channel_, h::serialize( var ) );
}

};  // namespace emlabcpp::testing
