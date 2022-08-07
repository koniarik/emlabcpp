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

#include "experimental/testing/reactor_interface_adapter.h"

using namespace emlabcpp;

void testing_reactor::spin( testing_reactor_interface& top_iface )
{
        testing_reactor_interface_adapter iface{ top_iface, seq_ };

        auto opt_var = iface.read_variant();

        if ( !opt_var ) {
                return;
        }

        apply_on_match(
            *opt_var,
            [&]( auto... args ) {
                    handle_message( args..., iface );
            },
            [&]( tag< TESTING_PARAM_VALUE >, const auto&... ) {
                    iface.report_failure< TESTING_UNDESIRED_MSG_E >();
            },
            [&]( tag< TESTING_PARAM_CHILD >, const auto&... ) {
                    iface.report_failure< TESTING_UNDESIRED_MSG_E >();
            },
            [&]( tag< TESTING_PARAM_CHILD_COUNT >, const auto&... ) {
                    iface.report_failure< TESTING_UNDESIRED_MSG_E >();
            },
            [&]( tag< TESTING_PARAM_KEY >, const auto&... ) {
                    iface.report_failure< TESTING_UNDESIRED_MSG_E >();
            },
            [&]( tag< TESTING_PARAM_TYPE >, const auto&... ) {
                    iface.report_failure< TESTING_UNDESIRED_MSG_E >();
            },
            [&]( tag< TESTING_TREE_ERROR >, const auto&... ) {
                    iface.report_failure< TESTING_UNDESIRED_MSG_E >();
            },
            [&]( tag< TESTING_COLLECT >, const auto&... ) {
                    iface.report_failure< TESTING_UNDESIRED_MSG_E >();
            } );
}

void testing_reactor::handle_message(
    tag< TESTING_SUITE_NAME >,
    testing_reactor_interface_adapter& iface )
{
        iface.reply< TESTING_SUITE_NAME >( testing_name_to_buffer( suite_name_ ) );
}
void testing_reactor::handle_message(
    tag< TESTING_SUITE_DATE >,
    testing_reactor_interface_adapter& iface )
{
        iface.reply< TESTING_SUITE_DATE >( testing_name_to_buffer( suite_date_ ) );
}
void testing_reactor::handle_message(
    tag< TESTING_COUNT >,
    testing_reactor_interface_adapter& iface )
{
        iface.reply< TESTING_COUNT >( static_cast< testing_test_id >( handles_.size() ) );
}
void testing_reactor::handle_message(
    tag< TESTING_NAME >,
    testing_test_id                    tid,
    testing_reactor_interface_adapter& iface )
{
        if ( tid >= handles_.size() ) {
                iface.report_failure< TESTING_BAD_TEST_ID_E >();
                return;
        }
        iface.reply< TESTING_NAME >( testing_name_to_buffer( access_test( tid ).name ) );
}
void testing_reactor::handle_message(
    tag< TESTING_LOAD >,
    testing_test_id                    tid,
    testing_run_id                     rid,
    testing_reactor_interface_adapter& iface )
{
        if ( active_exec_ ) {
                iface.report_failure< TESTING_TEST_ALREADY_LOADED_E >();
                return;
        }

        if ( tid >= handles_.size() ) {
                iface.report_failure< TESTING_BAD_TEST_ID_E >();
                return;
        }

        active_exec_ = active_execution{ tid, rid, &access_test( tid ) };
}
void testing_reactor::handle_message(
    tag< TESTING_EXEC >,
    testing_run_id,
    testing_reactor_interface_adapter& iface )
{
        if ( !active_exec_ ) {
                iface.report_failure< TESTING_TEST_NOT_LOADED_E >();
                return;
        }

        exec_test( iface );
        active_exec_.reset();
}

void testing_reactor::exec_test( testing_reactor_interface_adapter& iface )
{
        defer d = [&] {
                iface.reply< TESTING_FINISHED >( active_exec_->rid );
        };

        test_handle& h = *active_exec_->handle_ptr;

        testing_record rec{ active_exec_->tid, active_exec_->rid, iface };

        h.ptr->setup( rec );
        if ( rec.errored() ) {
                iface.reply< TESTING_FAILURE >( active_exec_->rid );
                return;
        }

        h.ptr->run( rec );
        if ( rec.errored() ) {
                iface.reply< TESTING_ERROR >( active_exec_->rid );
        }

        h.ptr->teardown( rec );
        if ( rec.errored() ) {
                iface.reply< TESTING_FAILURE >( active_exec_->rid );
        }
}

testing_reactor::test_handle& testing_reactor::access_test( testing_test_id tid )
{
        auto iter = handles_.begin();
        std::advance( iter, tid );
        return *iter;
}

testing_reactor::~testing_reactor()
{
        for ( test_handle& h : handles_ ) {
                std::destroy_at( h.ptr );
                mem_->deallocate( h.ptr );
        }
}
