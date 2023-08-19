///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
/// and associated documentation files (the "Software"), to deal in the Software without
/// restriction, including without limitation the rights to use, copy, modify, merge, publish,
/// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or
/// substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
/// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
/// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

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
                // TODO: this should not be ignored
                std::ignore = iface_.reply( test_finished{
                    .rid     = opt_exec_->get_run_id(),
                    .errored = opt_exec_->errored(),
                    .failed  = opt_exec_->failed() } );
                opt_exec_.reset();
        }
}

outcome reactor::on_msg( const std::span< const std::byte > buffer )
{
        using h = protocol::handler< controller_reactor_group >;
        return h::extract( view_n( buffer.data(), buffer.size() ) )
            .match(
                [this]( const controller_reactor_variant& var ) {
                        return on_msg( var );
                },
                [this]( const protocol::error_record& rec ) -> outcome {
                        // error is returned anyway
                        std::ignore = iface_.report_failure( input_message_protocol_error{ rec } );
                        return ERROR;
                } );
}

outcome reactor::on_msg( const controller_reactor_variant& var )
{
        return match( var, [this]( const auto& item ) {
                return handle_message( item );
        } );
}

outcome reactor::handle_message( const get_property< msgid::SUITE_NAME > )
{
        return iface_.reply( get_suite_name_reply{ name_buffer( suite_name_ ) } );
}

outcome reactor::handle_message( const get_property< msgid::SUITE_DATE > )
{
        return iface_.reply( get_suite_date_reply{ name_buffer( suite_date_ ) } );
}

outcome reactor::handle_message( const get_property< msgid::COUNT > )
{
        const std::size_t c = root_node_.count_next();
        return iface_.reply( get_count_reply{ static_cast< test_id >( c ) } );
}

outcome reactor::handle_message( const get_test_name_request req )
{
        test_ll_node* const node_ptr = root_node_.get_next( req.tid + 1 );
        if ( node_ptr == nullptr ) {
                std::ignore = iface_.report_failure( error< BAD_TEST_ID_E >{} );
                return FAILURE;
        }
        const test_interface& test = **node_ptr;
        return iface_.reply( get_test_name_reply{ name_buffer( test.get_name() ) } );
}

outcome reactor::handle_message( const exec_request req )
{
        if ( opt_exec_ ) {
                std::ignore = iface_.report_failure( error< TEST_IS_RUNING_E >{} );
                return FAILURE;
        }

        test_ll_node* const node_ptr = root_node_.get_next( req.tid + 1 );
        if ( node_ptr == nullptr ) {
                std::ignore = iface_.report_failure( error< BAD_TEST_ID_E >{} );
                return FAILURE;
        }
        test_interface& test = **node_ptr;
        opt_exec_.emplace( req.rid, mem_, test );
        return SUCCESS;
}
}  // namespace emlabcpp::testing
