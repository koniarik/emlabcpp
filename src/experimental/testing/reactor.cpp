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

void reactor::tick()
{
        if ( !opt_exec_.has_value() ) {
                return;
        }
        opt_exec_->tick();
        if ( opt_exec_->finished() ) {
                iface_.reply( test_finished{
                    .rid     = opt_exec_->get_run_id(),
                    .errored = opt_exec_->errored(),
                    .failed  = opt_exec_->failed() } );
                opt_exec_.reset();
        }
}
void reactor::on_msg( std::span< const uint8_t > buffer )
{
        using h = protocol::handler< controller_reactor_group >;
        h::extract( view_n( buffer.data(), buffer.size() ) )
            .match(
                [this]( const controller_reactor_variant& var ) {
                        on_msg( var );
                },
                [this]( const protocol::error_record& rec ) {
                        iface_.report_failure( input_message_protocol_error{ rec } );
                } );
}
void reactor::on_msg( const controller_reactor_variant& var )
{
        match( var, [this]( const auto& item ) {
                handle_message( item );
        } );
}

void reactor::handle_message( const get_property< SUITE_NAME > )
{
        iface_.reply( get_suite_name_reply{ name_to_buffer( suite_name_ ) } );
}
void reactor::handle_message( const get_property< SUITE_DATE > )
{
        iface_.reply( get_suite_date_reply{ name_to_buffer( suite_date_ ) } );
}
void reactor::handle_message( const get_property< COUNT > )
{
        const std::size_t c = root_test_.count_next();
        iface_.reply( get_count_reply{ static_cast< test_id >( c ) } );
}
void reactor::handle_message( const get_test_name_request req )
{
        test_interface* const test_ptr = root_test_.get_next( req.tid + 1 );
        if ( test_ptr == nullptr ) {
                iface_.report_failure( error< BAD_TEST_ID_E >{} );
                return;
        }
        iface_.reply( get_test_name_reply{ test_ptr->name } );
}

void reactor::handle_message( const exec_request req )
{
        if ( opt_exec_ ) {
                iface_.report_failure( error< TEST_IS_RUNING_E >{} );
                return;
        }

        test_interface* const test_ptr = root_test_.get_next( req.tid + 1 );
        if ( test_ptr == nullptr ) {
                iface_.report_failure( error< BAD_TEST_ID_E >{} );
                return;
        }

        opt_exec_.emplace( req.rid, mem_, *test_ptr );
}
}  // namespace emlabcpp::testing
