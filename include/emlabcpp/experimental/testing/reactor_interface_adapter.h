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

#include "emlabcpp/experimental/multiplexer.h"
#include "emlabcpp/experimental/testing/protocol.h"
#include "emlabcpp/static_function.h"

#include <utility>

#pragma once

namespace emlabcpp::testing
{
class reactor_interface_adapter
{
        using incoming_handler = static_function< bool( const controller_reactor_variant& ), 16 >;

        protocol::channel_type    channel_;
        reactor_transmit_callback transmit_;
        incoming_handler          h_{};

public:
        reactor_interface_adapter(
            const protocol::channel_type chann,
            reactor_transmit_callback    tb )
          : channel_( chann )
          , transmit_( std::move( tb ) )
        {
        }

        void reply( const reactor_controller_variant& );

        void report_failure( const reactor_error_variant& );
};
}  // namespace emlabcpp::testing
