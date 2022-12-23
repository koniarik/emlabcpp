
#include "emlabcpp/experimental/testing/parameters.h"

namespace emlabcpp::testing
{
bool param_type_processor::set_value( const server_client_params_variant& var )
{
        const auto* const val_ptr = std::get_if< param_type_reply >( &var );
        if ( val_ptr == nullptr ) {
                return false;
        }
        reply = val_ptr->type;
        return true;
}
template class params_awaiter< param_type_processor >;

[[nodiscard]] bool param_child_processor::set_value( const server_client_params_variant& var )
{
        const auto* const val_ptr = std::get_if< param_child_reply >( &var );
        if ( val_ptr == nullptr ) {
                return false;
        }
        reply = val_ptr->chid;
        return true;
}
template class params_awaiter< param_child_processor >;

[[nodiscard]] bool param_child_count_processor::set_value( const server_client_params_variant& var )
{
        const auto* const val_ptr = std::get_if< param_child_count_reply >( &var );
        if ( val_ptr == nullptr ) {
                return false;
        }
        reply = val_ptr->count;
        return true;
}
template class params_awaiter< param_child_count_processor >;

[[nodiscard]] bool param_key_processor::set_value( const server_client_params_variant& var )
{
        const auto* const val_ptr = std::get_if< param_key_reply >( &var );
        if ( val_ptr == nullptr ) {
                return false;
        }
        reply = val_ptr->key;
        return true;
}
template class params_awaiter< param_key_processor >;

parameters::parameters( params_send_callback send_cb )
  : send_cb_( std::move( send_cb ) )
{
}

void parameters::on_msg( std::span< const uint8_t > data )
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

void parameters::on_msg( const server_client_params_variant& req )
{
        if ( !reply_cb_ ) {
                return;
        }
        reply_cb_( req );
}

param_type_awaiter parameters::get_type( node_id nid )
{
        return param_type_awaiter{ param_type_request{ nid }, *this };
}

param_child_awaiter parameters::get_child( node_id nid, child_id chid )
{
        return param_child_awaiter{ param_child_request{ nid, chid }, *this };
}
param_child_awaiter parameters::get_child( node_id nid, const key_type& key )
{
        return param_child_awaiter{ param_child_request{ nid, key }, *this };
}

param_child_awaiter parameters::get_child( node_id nid, std::string_view key )
{
        return get_child( nid, key_type_to_buffer( key ) );
}

param_child_count_awaiter parameters::get_child_count( const node_id nid )
{
        return param_child_count_awaiter{ param_child_count_request{ .parent = nid }, *this };
}

param_key_awaiter parameters::get_key( node_id nid, child_id chid )
{
        return param_key_awaiter{ param_key_request{ nid, chid }, *this };
}

void parameters::exchange( const client_server_params_variant& req, params_reply_callback reply_cb )
{
        reply_cb_ = reply_cb;
        send( req );
}

void parameters::send( const client_server_params_variant& val )
{
        using h = protocol::handler< client_server_params_group >;
        send_cb_( h::serialize( val ) );
}

parameters_server::parameters_server( data_tree tree, params_send_callback send_cb )
  : tree_( tree )
  , send_cb_( std::move( send_cb ) )
{
}

void parameters_server::on_msg( std::span< const uint8_t > data )
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

void parameters_server::on_msg( const client_server_params_variant& req )
{
        visit(
            [&]( const auto& item ) {
                    on_req( item );
            },
            req );
}

void parameters_server::on_req( const param_value_request& req )
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
void parameters_server::on_req( const param_value_key_request& req )
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
void parameters_server::on_req( const param_child_request& req )
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
void parameters_server::on_req( const param_child_count_request& req )
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
void parameters_server::on_req( const param_key_request& req )
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
void parameters_server::on_req( const param_type_request& req )
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

void parameters_server::reply_node_error(
    const contiguous_request_adapter_errors_enum err,
    const node_id                                nid )
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

void parameters_server::send( const server_client_params_variant& var )
{
        using h = protocol::handler< server_client_params_group >;
        send_cb_( h::serialize( var ) );
}

};  // namespace emlabcpp::testing
