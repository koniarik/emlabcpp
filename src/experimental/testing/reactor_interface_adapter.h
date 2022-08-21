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

#include "emlabcpp/experimental/testing/reactor_interface.h"

#pragma once

namespace emlabcpp::testing
{
class testing_reactor_interface_adapter
{
        testing_reactor_interface&                         iface_;
        testing_controller_reactor_packet::sequencer_type& seq_;

        static constexpr std::size_t read_limit_ = 10;

public:
        testing_reactor_interface_adapter(
            testing_reactor_interface&                         iface,
            testing_controller_reactor_packet::sequencer_type& seq )
          : iface_( iface )
          , seq_( seq )
        {
        }

        testing_reactor_interface& operator*()
        {
                return iface_;
        }

        testing_reactor_interface* operator->()
        {
                return &iface_;
        }

        std::optional< testing_controller_reactor_variant > read_variant();

        void reply( const testing_reactor_controller_variant& );

        template < messages_enum ID, typename... Args >
        void reply( const Args&... args )
        {
                reply( testing_reactor_controller_group::make_val< ID >( args... ) );
        }

        void report_failure( const testing_reactor_error_variant& );

        template < testing_error_enum ID, typename... Args >
        void report_failure( const Args&... args )
        {
                report_failure( testing_reactor_error_group::make_val< ID >( args... ) );
        }
};
}  // namespace emlabcpp::testing
