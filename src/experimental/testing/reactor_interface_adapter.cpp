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
#include "experimental/testing/reactor_interface_adapter.h"

namespace emlabcpp::testing
{

std::optional< controller_reactor_variant >
testing_reactor_interface_adapter::read_variant()
{
        using sequencer = std::decay_t< decltype( seq_ ) >;

        uint8_t                                         to_read = sequencer::fixed_size;
        std::optional< controller_reactor_msg > opt_msg;
        while ( !opt_msg ) {
                std::optional opt_data = iface_.receive( to_read );
                if ( !opt_data ) {
                        return {};
                }

                seq_.load_data( view{ *opt_data } )
                    .match(
                        [&]( std::size_t next_read ) {
                                to_read = static_cast< uint8_t >( next_read );
                        },
                        [&]( auto msg ) {
                                opt_msg = msg;
                        } );
        }

        if ( !opt_msg ) {
                return {};
        }

        std::optional< controller_reactor_variant > res;
        controller_reactor_extract( *opt_msg )
            .match(
                [&]( auto var ) {
                        res = var;
                },
                [&]( protocol::error_record rec ) {
                        reply< TESTING_PROTOCOL_ERROR >( rec );
                } );
        return res;
}
void testing_reactor_interface_adapter::reply( const reactor_controller_variant& var )
{
        auto msg = reactor_controller_serialize( var );
        iface_.transmit( msg );
}

void testing_reactor_interface_adapter::report_failure( const testing_reactor_error_variant& evar )
{
        reply< TESTING_INTERNAL_ERROR >( evar );
}
}  // namespace emlabcpp::testing
