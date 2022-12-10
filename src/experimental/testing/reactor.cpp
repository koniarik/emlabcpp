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
#include "emlabcpp/experimental/testing/reactor.h"

#include "emlabcpp/experimental/testing/reactor_interface_adapter.h"

namespace emlabcpp::testing
{

test_interface& reactor::get_first_dummy_test()
{
        return root_test_;
}

void reactor::spin( reactor_interface& top_iface )
{
        reactor_interface_adapter iface{ top_iface, ep_ };

        iface.read_variant().match(
            [&]( const controller_reactor_variant& var ) {
                    // TODO: this does not work, split the CR group into multiple groups...
                    match(
                        var,
                        [&]( const auto& item ) {
                                handle_message( item, iface );
                        },
                        [&]( const param_value_reply& ) {
                                iface.report_failure( error< UNDESIRED_MSG_E >{} );
                        },
                        [&]( const param_value_key_reply& ) {
                                iface.report_failure( error< UNDESIRED_MSG_E >{} );
                        },
                        [&]( const param_child_reply& ) {
                                iface.report_failure( error< UNDESIRED_MSG_E >{} );
                        },
                        [&]( const param_child_count_reply& ) {
                                iface.report_failure( error< UNDESIRED_MSG_E >{} );
                        },
                        [&]( const param_key_reply& ) {
                                iface.report_failure( error< UNDESIRED_MSG_E >{} );
                        },
                        [&]( const param_type_reply& ) {
                                iface.report_failure( error< UNDESIRED_MSG_E >{} );
                        },
                        [&]( const tree_error_reply& ) {
                                iface.report_failure( error< UNDESIRED_MSG_E >{} );
                        },
                        [&]( const collect_reply& ) {
                                iface.report_failure( error< UNDESIRED_MSG_E >{} );
                        } );
            },
            [&]( const protocol::endpoint_error& e ) {
                    match(
                        e,
                        [&]( const protocol::endpoint_load_error& ) {
                                iface.report_failure( no_response_error{} );
                        },
                        [&]( const protocol::error_record& rec ) {
                                iface.report_failure( input_message_protocol_error{ rec } );
                        } );
            } );
}

void reactor::handle_message( get_property< SUITE_NAME >, reactor_interface_adapter& iface )
{
        iface.reply( get_suite_name_reply{ name_to_buffer( suite_name_ ) } );
}
void reactor::handle_message( get_property< SUITE_DATE >, reactor_interface_adapter& iface )
{
        iface.reply( get_suite_date_reply{ name_to_buffer( suite_date_ ) } );
}
void reactor::handle_message( get_property< COUNT >, reactor_interface_adapter& iface )
{
        std::size_t c = root_test_.count_next();
        iface.reply( get_count_reply{ static_cast< test_id >( c ) } );
}
void reactor::handle_message( get_test_name_request req, reactor_interface_adapter& iface )
{
        test_interface* test_ptr = root_test_.get_next( req.tid + 1 );
        if ( test_ptr == nullptr ) {
                iface.report_failure( error< BAD_TEST_ID_E >{} );
                return;
        }
        iface.reply( get_test_name_reply{ test_ptr->name } );
}
void reactor::handle_message( load_test req, reactor_interface_adapter& iface )
{
        if ( active_exec_ ) {
                iface.report_failure( error< TEST_ALREADY_LOADED_E >{} );
                return;
        }

        test_interface* test_ptr = root_test_.get_next( req.tid + 1 );
        if ( test_ptr == nullptr ) {
                iface.report_failure( error< BAD_TEST_ID_E >{} );
                return;
        }

        active_exec_ = active_execution{ req.tid, req.rid, test_ptr };
}
void reactor::handle_message( exec_request, reactor_interface_adapter& iface )
{
        if ( !active_exec_ ) {
                iface.report_failure( error< TEST_NOT_LOADED_E >{} );
                return;
        }

        exec_test( iface );
        active_exec_.reset();
}

void reactor::exec_test( reactor_interface_adapter& iface )
{

        test_interface* test = active_exec_->iface_ptr;
        record          rec{ active_exec_->tid, active_exec_->rid, iface };
        bool            errd  = false;
        bool            faild = false;

        defer d = [&] {
                iface.reply(
                    test_finished{ .rid = active_exec_->rid, .errored = errd, .failed = faild } );
        };

        {
                test_coroutine coro = test->setup( &mem_, rec );
                if ( !coro.spin( &iface ) ) {
                        return;
                }
        }

        if ( rec.errored() ) {
                faild = true;
                return;
        }

        {
                test_coroutine coro = test->run( &mem_, rec );
                if ( !coro.spin( &iface ) ) {
                        return;
                }
        }

        {
                test_coroutine coro = test->teardown( &mem_, rec );
                if ( !coro.spin( &iface ) ) {
                        return;
                }
        }

        if ( rec.errored() ) {
                errd = true;
                return;
        }
}

}  // namespace emlabcpp::testing
