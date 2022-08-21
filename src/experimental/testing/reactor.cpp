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

namespace emlabcpp::testing
{

// TODO maybe generalize?

void reactor::spin( reactor_interface& top_iface )
{
        reactor_interface_adapter iface{ top_iface, seq_ };

        auto opt_var = iface.read_variant();

        if ( !opt_var ) {
                return;
        }

        // TODO: this does not work, split the CR group into multiple groups...
        match(
            *opt_var,
            [&]( const auto& item ) {
                    handle_message( item, iface );
            },
            [&]( const param_value_reply& ) {
                    iface.report_failure< UNDESIRED_MSG_E >();
            },
            [&]( const param_child_reply& ) {
                    iface.report_failure< UNDESIRED_MSG_E >();
            },
            [&]( const param_child_count_reply& ) {
                    iface.report_failure< UNDESIRED_MSG_E >();
            },
            [&]( const param_key_reply& ) {
                    iface.report_failure< UNDESIRED_MSG_E >();
            },
            [&]( const param_type_reply& ) {
                    iface.report_failure< UNDESIRED_MSG_E >();
            },
            [&]( const tree_error_reply& ) {
                    iface.report_failure< UNDESIRED_MSG_E >();
            },
            [&]( const collect_reply& ) {
                    iface.report_failure< UNDESIRED_MSG_E >();
            } );
}

void reactor::handle_message( get_property< SUITE_NAME >, reactor_interface_adapter& iface )
{
        iface.reply< SUITE_NAME >( name_to_buffer( suite_name_ ) );
}
void reactor::handle_message( get_property< SUITE_DATE >, reactor_interface_adapter& iface )
{
        iface.reply< SUITE_DATE >( name_to_buffer( suite_date_ ) );
}
void reactor::handle_message( get_property< COUNT >, reactor_interface_adapter& iface )
{
        iface.reply< COUNT >( static_cast< test_id >( handles_.size() ) );
}
void reactor::handle_message( get_test_name req, reactor_interface_adapter& iface )
{
        if ( req.tid >= handles_.size() ) {
                iface.report_failure< BAD_TEST_ID_E >();
                return;
        }
        iface.reply< NAME >( name_to_buffer( access_test( req.tid ).name ) );
}
void reactor::handle_message( load_test req, reactor_interface_adapter& iface )
{
        if ( active_exec_ ) {
                iface.report_failure< TEST_ALREADY_LOADED_E >();
                return;
        }

        if ( req.tid >= handles_.size() ) {
                iface.report_failure< BAD_TEST_ID_E >();
                return;
        }

        active_exec_ = active_execution{ req.tid, req.rid, &access_test( req.tid ) };
}
void reactor::handle_message( exec_request, reactor_interface_adapter& iface )
{
        if ( !active_exec_ ) {
                iface.report_failure< TEST_NOT_LOADED_E >();
                return;
        }

        exec_test( iface );
        active_exec_.reset();
}

void reactor::exec_test( reactor_interface_adapter& iface )
{
        defer d = [&] {
                iface.reply< FINISHED >( active_exec_->rid );
        };

        test_handle& h = *active_exec_->handle_ptr;

        record rec{ active_exec_->tid, active_exec_->rid, iface };

        h.ptr->setup( rec );
        if ( rec.errored() ) {
                iface.reply< FAILURE >( active_exec_->rid );
                return;
        }

        h.ptr->run( rec );
        bool run_errored = rec.errored();

        h.ptr->teardown( rec );

        if ( run_errored ) {
                iface.reply< ERROR >( active_exec_->rid );
        } else if ( rec.errored() ) {
                iface.reply< FAILURE >( active_exec_->rid );
        }
}

reactor::test_handle& reactor::access_test( test_id tid )
{
        auto iter = handles_.begin();
        std::advance( iter, tid );
        return *iter;
}

reactor::~reactor()
{
        for ( test_handle& h : handles_ ) {
                std::destroy_at( h.ptr );
                mem_->deallocate( h.ptr, h.alignment );
        }
}

}  // namespace emlabcpp::testing
