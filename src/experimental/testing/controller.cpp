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

namespace emlabcpp::testing
{

class controller_interface_adapter
{
        controller_interface& iface_;
        controller_endpoint&  ep_;

        static constexpr std::size_t read_limit_ = 10;

public:
        explicit controller_interface_adapter(
            controller_interface& iface,
            controller_endpoint&  ep )
          : iface_( iface )
          , ep_( ep )
        {
        }

        void send( const controller_reactor_variant& var )
        {
                auto msg = ep_.serialize( var );
                iface_.transmit( msg );
        }

        controller_interface* operator->()
        {
                return &iface_;
        }

        controller_interface& operator*()
        {
                return iface_;
        }

        either< reactor_controller_variant, protocol::endpoint_error > read_variant()
        {
                return ep_.load_variant( read_limit_, [this]( const std::size_t c ) {
                        return this->iface_.receive( c );
                } );
        }
        void reply_node_error(
            const run_id                                 rid,
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
                send( tree_error_reply{ .rid = rid, .err = err, .nid = nid } );
        }

        void report_error( const error_variant& var )
        {
                log_error( var );
                iface_.on_error( var );
        }
#ifdef EMLABCPP_USE_LOGGING
        void log_error( const error_variant& var )
        {
                match(
                    var,
                    []( const reactor_protocol_error& e ) {
                            EMLABCPP_LOG( "Protocol error reported from reactor: " << e );
                    },
                    []( const controller_protocol_error& e ) {
                            EMLABCPP_LOG( "Protocol error reported from controller: " << e );
                    },
                    []( const internal_reactor_error& e ) {
                            EMLABCPP_LOG( "Internal error from reactor: " << e );
                    },
                    []( const controller_internal_error& e ) {
                            EMLABCPP_LOG( "Wrong message arrived to controller: " << e );
                    } );
                const auto* const internal_ptr = std::get_if< internal_reactor_error >( &var );

                if ( internal_ptr != nullptr ) {
                        match(
                            internal_ptr->val,
                            [this]( const wrong_type_error& err ) {
                                    const auto* const node_ptr =
                                        this->iface_.get_param_tree().get_node( err.nid );
                                    EMLABCPP_LOG(
                                        "Error due a wrong type of node "
                                        << err.nid << " asserted in reactors test: " << *node_ptr );
                            },
                            []( const auto& ) {} );
                }
        }
#else
        void log_error( const error_variant& )
        {
        }
#endif
};

namespace
{
        template < typename ReturnType, typename Req >
        std::optional< ReturnType > load_data( const Req& req, controller_interface_adapter& iface )
        {
                std::optional< ReturnType > res;

                iface.send( req );
                iface.read_variant().match(
                    [&iface, &res]( const reactor_controller_variant& var ) {
                            auto handle = matcher{
                                [&res]( const ReturnType& item ) {
                                        res = item;
                                },
                                [&iface]( const reactor_internal_error_report& err ) {
                                        iface.report_error( internal_reactor_error{ err.var } );
                                },
                                [&iface]< typename T >( const T& ) {
                                        iface.report_error( controller_internal_error{ T::id } );
                                } };

                            // TODO: log won't work on embedded
                            // EMLABCPP_DEBUG_LOG( "con<-rec: " << var );
                            visit( handle, var );
                    },
                    []( const protocol::endpoint_error& ) {} );
                return res;
        }
}  // namespace

std::optional< controller >
controller::make( controller_interface& top_iface, pmr::memory_resource& mem_resource )
{
        controller_endpoint          ep;
        controller_interface_adapter iface{ top_iface, ep };

        std::optional opt_name =
            load_data< get_suite_name_reply >( get_property< SUITE_NAME >{}, iface );
        std::optional opt_date =
            load_data< get_suite_date_reply >( get_property< SUITE_DATE >{}, iface );
        std::optional opt_count = load_data< get_count_reply >( get_property< COUNT >{}, iface );

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

        pmr::map< test_id, test_info > info{ mem_resource };

        for ( test_id i = 0; i < opt_count->count; i++ ) {
                auto opt_test_name =
                    load_data< get_test_name_reply >( get_test_name_request{ .tid = i }, iface );
                if ( !opt_test_name ) {
                        EMLABCPP_LOG(
                            "Failed to build controller - did not get a test name for index: "
                            << i );
                        return {};
                }
                info[i] = test_info{ .name = opt_test_name->name };
        }

        return controller{ opt_name->name, opt_date->date, std::move( info ), mem_resource };
}

void controller::start_test( test_id tid, controller_interface& top_iface )
{
        controller_interface_adapter iface{ top_iface, ep_ };
        EMLABCPP_ASSERT( !context_ );

        rid_ += 1;

        context_.emplace( tid, rid_, mem_res_ );

        iface.send( load_test{ tid, rid_ } );
        iface.send( exec_request{ rid_ } );
}

struct controller_dispatcher
{
        controller_interface_adapter& iface;
        std::optional< test_result >& context;

        void operator()( const get_count_reply& )
        {
                iface.report_error( controller_internal_error{ COUNT } );
        }

        void operator()( const get_test_name_reply& )
        {
                iface.report_error( controller_internal_error{ NAME } );
        }
        void operator()( const collect_request& req )
        {
                EMLABCPP_ASSERT( context );
                EMLABCPP_ASSERT( context->rid == req.rid );  // TODO better error handling

                data_tree& tree = context->collected;

                // TODO: this may be a bad idea ...
                if ( tree.empty() ) {
                        if ( req.opt_key ) {
                                EMLABCPP_LOG( "collect tree is empty, making root object" );
                                tree.make_object_node();
                        } else {
                                EMLABCPP_LOG( "collect tree is empty, making root array" );
                                tree.make_array_node();
                        }
                }

                contiguous_request_adapter harn{ tree };

                either< node_id, contiguous_request_adapter_errors_enum > res =
                    req.opt_key ? harn.insert( req.parent, *req.opt_key, req.value ) :
                                  harn.insert( req.parent, req.value );
                res.match(
                    [this, &req]( const node_id nid ) {
                            if ( !req.expects_reply ) {
                                    return;
                            }
                            this->iface.send( collect_reply{ req.rid, nid } );
                    },
                    [this, &req]( const contiguous_request_adapter_errors_enum err ) {
                            this->iface.reply_node_error( req.rid, err, req.parent );
                    } );
        }
        void operator()( const param_value_request& req )
        {
                EMLABCPP_ASSERT( context );
                EMLABCPP_ASSERT( context->rid == req.rid );  // TODO better error handling

                const contiguous_request_adapter harn{ iface->get_param_tree() };

                harn.get_value( req.nid ).match(
                    [this, &req]( const value_type& val ) {
                            this->iface.send( param_value_reply{ req.rid, val } );
                    },
                    [this, &req]( const contiguous_request_adapter_errors_enum err ) {
                            this->iface.reply_node_error( req.rid, err, req.nid );
                    } );
        }
        void operator()( const param_value_key_request& req )
        {
                EMLABCPP_ASSERT( context );
                EMLABCPP_ASSERT( context->rid == req.rid );  // TODO better error handling

                const contiguous_request_adapter harn{ iface->get_param_tree() };

                harn.get_child( req.nid, req.key )
                    .bind_left( [this, &req, &harn]( const child_id chid ) {
                            return harn.get_value( chid );
                    } )
                    .match(
                        [this, &req]( const value_type& val ) {
                                this->iface.send( param_value_key_reply{ req.rid, val } );
                        },
                        [this, &req]( const contiguous_request_adapter_errors_enum err ) {
                                this->iface.reply_node_error( req.rid, err, req.nid );
                        } );
        }
        void operator()( const param_child_request& req )
        {
                EMLABCPP_ASSERT( context );
                EMLABCPP_ASSERT( context->rid == req.rid );  // TODO better error handling

                const contiguous_request_adapter harn{ iface->get_param_tree() };

                harn.get_child( req.parent, req.chid )
                    .match(
                        [this, &req]( const node_id child ) {
                                this->iface.send( param_child_reply{ req.rid, child } );
                        },
                        [this, &req]( const contiguous_request_adapter_errors_enum err ) {
                                this->iface.reply_node_error( req.rid, err, req.parent );
                        } );
        }
        void operator()( const param_child_count_request& req )
        {
                EMLABCPP_ASSERT( context );
                EMLABCPP_ASSERT( context->rid == req.rid );  // TODO better error handling

                const contiguous_request_adapter harn{ iface->get_param_tree() };

                harn.get_child_count( req.parent )
                    .match(
                        [this, &req]( const child_id count ) {
                                this->iface.send( param_child_count_reply{ req.rid, count } );
                        },
                        [this, &req]( const contiguous_request_adapter_errors_enum err ) {
                                this->iface.reply_node_error( req.rid, err, req.parent );
                        } );
        }
        void operator()( const param_key_request& req )
        {
                EMLABCPP_ASSERT( context );
                EMLABCPP_ASSERT( context->rid == req.rid );  // TODO better error handling

                const contiguous_request_adapter harn{ iface->get_param_tree() };

                harn.get_key( req.nid, req.chid )
                    .match(
                        [this, &req]( const key_type& key ) {
                                iface.send( param_key_reply{ req.rid, key } );
                        },
                        [this, &req]( const contiguous_request_adapter_errors_enum err ) {
                                iface.reply_node_error( req.rid, err, req.nid );
                        } );
        }
        void operator()( const param_type_request& req )
        {
                EMLABCPP_ASSERT( context );
                EMLABCPP_ASSERT( context->rid == req.rid );  // TODO better error handling

                const contiguous_request_adapter harn{ iface->get_param_tree() };

                harn.get_type( req.nid ).match(
                    [this, &req]( const contiguous_tree_type_enum type ) {
                            iface.send( param_type_reply{ req.rid, type } );
                    },
                    [this, &req]( const contiguous_request_adapter_errors_enum err ) {
                            iface.reply_node_error( req.rid, err, req.nid );
                    } );
        }
        void operator()( const test_finished& req )
        {
                EMLABCPP_ASSERT( context );
                EMLABCPP_ASSERT( context->rid == req.rid );  // TODO better error handling

                context->failed  = req.failed;
                context->errored = req.errored;
                iface->on_result( *context );
                context.reset();
        }
        void operator()( const get_suite_name_reply& )
        {
                iface.report_error( controller_internal_error{ SUITE_NAME } );
        }
        void operator()( const get_suite_date_reply& )
        {
                iface.report_error( controller_internal_error{ SUITE_DATE } );
        }
        void operator()( const reactor_internal_error_report& report )
        {
                iface.report_error( internal_reactor_error{ report.var } );
        }
};

void controller::tick( controller_interface& top_iface )
{
        controller_interface_adapter iface{ top_iface, ep_ };

        if ( !context_ ) {
                return;
        }

        iface.read_variant().match(
            [this, &iface]( const reactor_controller_variant& var ) {
                    // TODO: won't work with embedded logging
                    // EMLABCPP_DEBUG_LOG( "con<-rec: " << var );
                    visit( controller_dispatcher{ iface, this->context_ }, var );
            },
            []( const protocol::endpoint_error& ) {} );
}
}  // namespace emlabcpp::testing
