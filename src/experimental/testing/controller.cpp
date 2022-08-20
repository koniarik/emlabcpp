// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//
//  Copyright © 2022 Jan Veverak Koniarik
//  This file is part of project: emlabcpp
//

#include "emlabcpp/experimental/testing/controller.h"

#include "emlabcpp/experimental/contiguous_tree/request_adapter.h"
#include "emlabcpp/experimental/logging.h"

using namespace emlabcpp;

class emlabcpp::testing_controller_interface_adapter
{
        testing_controller_interface& iface_;

        static constexpr std::size_t read_limit_ = 10;

public:
        testing_controller_interface_adapter( testing_controller_interface& iface )
          : iface_( iface )
        {
        }

        void send( const testing_controller_reactor_variant& var )
        {
                EMLABCPP_DEBUG_LOG( "con->rec: " << var );
                auto msg = testing_controller_reactor_serialize( var );
                iface_.transmit( msg );
        }

        testing_controller_interface* operator->()
        {
                return &iface_;
        }

        testing_controller_interface& operator*()
        {
                return iface_;
        }

        std::optional< testing_reactor_controller_msg > read_message()
        {
                using sequencer = testing_reactor_controller_packet::sequencer;
                return protocol_simple_load< sequencer >( read_limit_, [&]( std::size_t c ) {
                        return iface_.receive( c );
                } );
        }
        void reply_node_error(
            testing_run_id                         rid,
            contiguous_request_adapter_errors_enum err,
            testing_node_id                        nid )
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
                send( testing_tree_error_reply{ .rid = rid, .err = err, .nid = nid } );
        }

        void report_error( testing_error_variant var )
        {
                log_error( var );
                iface_.on_error( std::move( var ) );
        }
#ifdef EMLABCPP_USE_LOGGING
        void log_error( const testing_error_variant& var )
        {
                match(
                    var,
                    []( const testing_reactor_protocol_error& e ) {
                            EMLABCPP_LOG( "Protocol error reported from reactor: " << e );
                    },
                    []( const testing_controller_protocol_error& e ) {
                            EMLABCPP_LOG( "Protocol error reported from controller: " << e );
                    },
                    []( const testing_internal_reactor_error& e ) {
                            EMLABCPP_LOG( "Internal error from reactor: " << e );
                    },
                    []( const testing_controller_message_error& e ) {
                            EMLABCPP_LOG( "Wrong message arrived to controller: " << e );
                    } );
                const auto* internal_ptr = std::get_if< testing_internal_reactor_error >( &var );

                if ( internal_ptr ) {
                        apply_on_match(
                            internal_ptr->val,
                            [&]( tag< TESTING_WRONG_TYPE_E >, auto nid ) {
                                    const testing_node* node_ptr =
                                        iface_.get_param_tree().get_node( nid );
                                    EMLABCPP_LOG(
                                        "Error due a wrong type of node "
                                        << nid << " asserted in reactors test: " << *node_ptr );
                            },
                            [&]< auto EID >( tag< EID >, const auto&... ){} );
                }
        }
#else
        void log_error( const testing_error_variant& )
        {
        }
#endif
};

namespace
{
template < typename T, typename Req >
std::optional< T > load_data( const Req& req, testing_controller_interface_adapter& iface )
{
        std::optional< T > res;

        iface.send( req );

        auto opt_msg = iface.read_message();
        if ( !opt_msg ) {
                return res;
        }

        auto handle = matcher{
            [&]( tag< Req::tag >, auto item ) {
                    res = item;
            },
            [&]( tag< TESTING_INTERNAL_ERROR >, testing_reactor_error_variant err ) {
                    iface.report_error( testing_internal_reactor_error{ std::move( err ) } );
            },
            [&]( tag< TESTING_PROTOCOL_ERROR >, protocol_error_record rec ) {
                    iface.report_error( testing_reactor_protocol_error{ rec } );
            },
            [&]< auto WID >( tag< WID >, auto... ){
                iface.report_error( testing_controller_message_error{ WID } );
}
};  // namespace

testing_reactor_controller_extract( *opt_msg )
    .match(
        [&]( auto pack ) {
                EMLABCPP_DEBUG_LOG( "con<-rec: " << pack );
                apply_on_visit( handle, pack );
        },
        [&]( protocol_error_record rec ) {
                EMLABCPP_LOG( "Protocol error from reactor: " << rec );
                iface.report_error( testing_controller_protocol_error{ rec } );
        } );
return res;
}
}

std::optional< testing_controller >
testing_controller::make( testing_controller_interface& top_iface, pool_interface* pool )
{
        testing_controller_interface_adapter iface{ top_iface };

        auto opt_name =
            load_data< testing_name_buffer >( testing_get_property< TESTING_SUITE_NAME >{}, iface );
        auto opt_date =
            load_data< testing_name_buffer >( testing_get_property< TESTING_SUITE_DATE >{}, iface );
        auto opt_count =
            load_data< testing_test_id >( testing_get_property< TESTING_COUNT >{}, iface );

        if ( !opt_name ) {
                EMLABCPP_LOG( "Failed to build controller - did not get a name" );
                return {};
        }
        if ( !opt_date ) {
                EMLABCPP_LOG( "Failed to build controller - did not get a date" );
                return {};
        }
        if ( !opt_count ) {
                EMLABCPP_LOG( "Failed to build controller - did not get a test count" );
                return {};
        }

        pool_map< testing_test_id, test_info > info{ pool };

        for ( testing_test_id i = 0; i < *opt_count; i++ ) {
                auto opt_name =
                    load_data< testing_name_buffer >( testing_get_test_name{ .tid = i }, iface );
                if ( !opt_name ) {
                        EMLABCPP_LOG(
                            "Failed to build controller - did not get a test name for index: "
                            << i );
                        return {};
                }
                info[i] = test_info{ .name = *opt_name };
        }

        return testing_controller{ *opt_name, *opt_date, std::move( info ), pool };
}
void testing_controller::handle_message(
    tag< TESTING_COUNT >,
    auto,
    testing_controller_interface_adapter& iface )
{
        iface.report_error( testing_controller_message_error{ TESTING_COUNT } );
}
void testing_controller::handle_message(
    tag< TESTING_NAME >,
    auto,
    testing_controller_interface_adapter& iface )
{
        iface.report_error( testing_controller_message_error{ TESTING_NAME } );
}
void testing_controller::handle_message(
    tag< TESTING_PARAM_VALUE >,
    testing_run_id                        rid,
    testing_node_id                       nid,
    testing_controller_interface_adapter& iface )
{
        EMLABCPP_ASSERT( context_ );
        EMLABCPP_ASSERT( context_->rid == rid );  // TODO better error handling

        contiguous_request_adapter harn{ iface->get_param_tree() };

        harn.get_value( nid ).match(
            [&]( const testing_value& val ) {
                    iface.send( testing_param_value_reply{ rid, val } );
            },
            [&]( contiguous_request_adapter_errors_enum err ) {
                    iface.reply_node_error( rid, err, nid );
            } );
}
void testing_controller::handle_message(
    tag< TESTING_PARAM_CHILD >,
    testing_run_id                                       rid,
    testing_node_id                                      nid,
    const std::variant< testing_key, testing_child_id >& chid,
    testing_controller_interface_adapter&                iface )
{
        EMLABCPP_ASSERT( context_ );
        EMLABCPP_ASSERT( context_->rid == rid );  // TODO better error handling

        contiguous_request_adapter harn{ iface->get_param_tree() };

        harn.get_child( nid, chid )
            .match(
                [&]( testing_node_id child ) {
                        iface.send( testing_param_child_reply{ rid, child } );
                },
                [&]( contiguous_request_adapter_errors_enum err ) {
                        iface.reply_node_error( rid, err, nid );
                } );
}
void testing_controller::handle_message(
    tag< TESTING_PARAM_CHILD_COUNT >,
    testing_run_id                        rid,
    testing_node_id                       nid,
    testing_controller_interface_adapter& iface )
{
        EMLABCPP_ASSERT( context_ );
        EMLABCPP_ASSERT( context_->rid == rid );  // TODO better error handling

        contiguous_request_adapter harn{ iface->get_param_tree() };

        harn.get_child_count( nid ).match(
            [&]( testing_child_id count ) {
                    iface.send( testing_param_child_count_reply{ rid, count } );
            },
            [&]( contiguous_request_adapter_errors_enum err ) {
                    iface.reply_node_error( rid, err, nid );
            } );
}
void testing_controller::handle_message(
    tag< TESTING_PARAM_KEY >,
    testing_run_id                        rid,
    testing_node_id                       nid,
    testing_child_id                      chid,
    testing_controller_interface_adapter& iface )
{
        EMLABCPP_ASSERT( context_ );
        EMLABCPP_ASSERT( context_->rid == rid );  // TODO better error handling

        contiguous_request_adapter harn{ iface->get_param_tree() };

        harn.get_key( nid, chid )
            .match(
                [&]( const testing_key& key ) {
                        iface.send( testing_param_key_reply{ rid, key } );
                },
                [&]( contiguous_request_adapter_errors_enum err ) {
                        iface.reply_node_error( rid, err, nid );
                } );
}
void testing_controller::handle_message(
    tag< TESTING_PARAM_TYPE >,
    testing_run_id                        rid,
    testing_node_id                       nid,
    testing_controller_interface_adapter& iface )
{
        EMLABCPP_ASSERT( context_ );
        EMLABCPP_ASSERT( context_->rid == rid );  // TODO better error handling

        contiguous_request_adapter harn{ iface->get_param_tree() };

        harn.get_type( nid ).match(
            [&]( contiguous_tree_type_enum type ) {
                    iface.send( testing_param_type_reply{ rid, type } );
            },
            [&]( contiguous_request_adapter_errors_enum err ) {
                    iface.reply_node_error( rid, err, nid );
            } );
}

void testing_controller::handle_message(
    tag< TESTING_COLLECT >,
    testing_run_id                        rid,
    testing_node_id                       parent,
    const std::optional< testing_key >&   opt_key,
    const testing_collect_arg&            val,
    testing_controller_interface_adapter& iface )
{
        EMLABCPP_ASSERT( context_ );
        EMLABCPP_ASSERT( context_->rid == rid );  // TODO better error handling

        testing_tree& tree = context_->collected;

        // TODO: this may be a bad idea ...
        if ( tree.empty() ) {
                if ( opt_key ) {
                        tree.make_object_node();
                } else {
                        tree.make_array_node();
                }
        }

        contiguous_request_adapter harn{ tree };

        either< testing_node_id, contiguous_request_adapter_errors_enum > res =
            opt_key ? harn.insert( parent, *opt_key, val ) : harn.insert( parent, val );
        res.match(
            [&]( testing_node_id nid ) {
                    iface.send( testing_collect_reply{ rid, nid } );
            },
            [&]( contiguous_request_adapter_errors_enum err ) {
                    iface.reply_node_error( rid, err, parent );
            } );
}
void testing_controller::handle_message(
    tag< TESTING_FINISHED >,
    auto,
    testing_controller_interface_adapter& iface )
{
        iface->on_result( *context_ );
        context_.reset();
}
void testing_controller::handle_message(
    tag< TESTING_ERROR >,
    auto,
    testing_controller_interface_adapter& )
{
        context_->errored = true;
}
void testing_controller::handle_message(
    tag< TESTING_FAILURE >,
    auto,
    testing_controller_interface_adapter& )
{
        context_->failed = true;
}
void testing_controller::handle_message(
    tag< TESTING_SUITE_NAME >,
    auto,
    testing_controller_interface_adapter& iface )
{
        iface.report_error( testing_controller_message_error{ TESTING_SUITE_NAME } );
}
void testing_controller::handle_message(
    tag< TESTING_SUITE_DATE >,
    auto,
    testing_controller_interface_adapter& iface )
{
        iface.report_error( testing_controller_message_error{ TESTING_SUITE_DATE } );
}
void testing_controller::handle_message(
    tag< TESTING_INTERNAL_ERROR >,
    testing_reactor_error_variant         err,
    testing_controller_interface_adapter& iface )
{
        iface.report_error( testing_internal_reactor_error{ std::move( err ) } );
}
void testing_controller::handle_message(
    tag< TESTING_PROTOCOL_ERROR >,
    protocol_error_record                 rec,
    testing_controller_interface_adapter& iface )
{
        iface.report_error( testing_reactor_protocol_error{ rec } );
}

void testing_controller::start_test( testing_test_id tid, testing_controller_interface& top_iface )
{
        testing_controller_interface_adapter iface{ top_iface };
        EMLABCPP_ASSERT( !context_ );

        rid_ += 1;

        context_.emplace( tid, rid_, mem_pool_ );

        iface.send( testing_load_test{ tid, rid_ } );
        iface.send( testing_exec{ rid_ } );
}

void testing_controller::tick( testing_controller_interface& top_iface )
{
        testing_controller_interface_adapter iface{ top_iface };

        if ( !context_ ) {
                return;
        }

        auto opt_msg = iface.read_message();
        if ( !opt_msg ) {
                return;
        }

        testing_reactor_controller_extract( *opt_msg )
            .match(
                [&]( auto var ) {
                        EMLABCPP_DEBUG_LOG( "con<-rec: " << var );
                        apply_on_visit(
                            [&]( auto... args ) {
                                    handle_message( args..., iface );
                            },
                            var );
                },
                [&]( protocol_error_record e ) {
                        iface.report_error( testing_controller_protocol_error{ e } );
                } );
}
