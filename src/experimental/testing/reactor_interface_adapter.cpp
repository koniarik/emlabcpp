/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
///

#include "emlabcpp/experimental/testing/reactor_interface_adapter.h"

#include "emlabcpp/experimental/testing/protocol.h"
#include "emlabcpp/protocol/handler.h"
#include "emlabcpp/result.h"

namespace emlabcpp::testing
{

result reactor_interface_adapter::reply( reactor_controller_variant const& var )
{
        using h        = protocol::handler< reactor_controller_group >;
        auto const msg = h::serialize( var );
        return transmit_( channel_, msg );
}

result reactor_interface_adapter::report_failure( reactor_error_variant const& evar )
{
        return reply( reactor_internal_error_report{ evar } );
}
}  // namespace emlabcpp::testing
