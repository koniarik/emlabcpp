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

either< controller_reactor_variant, protocol::endpoint_error >
reactor_interface_adapter::read_variant()
{
        // TODO: ugly constant
        return ep_.load_variant( 10, [this]( const std::size_t c ) {
                return iface_.receive( static_cast< uint8_t >( c ) );
        } );
}
void reactor_interface_adapter::reply( const reactor_controller_variant& var )
{
        auto msg = ep_.serialize( var );
        iface_.transmit( msg );
}

void reactor_interface_adapter::report_failure( const reactor_error_variant& evar )
{
        reply( reactor_internal_error_report{ evar } );
}
}  // namespace emlabcpp::testing
