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

#pragma once

#include "emlabcpp/experimental/testing/controller_interface.h"
#include "emlabcpp/static_function.h"

namespace emlabcpp::testing
{

class controller_interface_adapter
{
        protocol::channel_type channel_;
        controller_interface&  iface_;

        static_function< bool( const reactor_controller_variant& ), 32 > reply_cb_;
        controller_transmit_callback                                     send_cb_;

public:
        explicit controller_interface_adapter(
            const protocol::channel_type channel,
            controller_interface&        iface,
            controller_transmit_callback send_cb )
          : channel_( channel )
          , iface_( iface )
          , send_cb_( std::move( send_cb ) )
        {
        }

        result send( const controller_reactor_variant& var )
        {
                using h        = protocol::handler< controller_reactor_group >;
                const auto msg = h::serialize( var );
                return send_cb_( channel_, msg );
        }

        controller_interface* operator->()
        {
                return &iface_;
        }

        controller_interface& operator*()
        {
                return iface_;
        }

        void set_reply_cb( static_function< bool( const reactor_controller_variant& ), 32 > cb )
        {
                reply_cb_ = std::move( cb );
        }

        bool on_msg_with_cb( const reactor_controller_variant& var )
        {
                if ( !reply_cb_ )
                        return false;
                return reply_cb_( var );
        }

        void report_error( const error_variant& var )
        {
                iface_.on_error( var );
        }
};

}  // namespace emlabcpp::testing
