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

#pragma once

#include "../../static_function.h"
#include "../multiplexer.h"
#include "./protocol.h"

#include <utility>

namespace emlabcpp::testing
{
class reactor_interface_adapter
{
        using incoming_handler = static_function< bool( controller_reactor_variant const& ), 16 >;

        protocol::channel_type    channel_;
        reactor_transmit_callback transmit_;
        incoming_handler          h_{};

public:
        reactor_interface_adapter(
            protocol::channel_type const chann,
            reactor_transmit_callback    tb )
          : channel_( chann )
          , transmit_( std::move( tb ) )
        {
        }

        result reply( reactor_controller_variant const& );

        result report_failure( reactor_error_variant const& );
};
}  // namespace emlabcpp::testing
