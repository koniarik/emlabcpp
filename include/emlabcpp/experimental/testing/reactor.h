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

#include "emlabcpp/experimental/testing/executor.h"
#include "emlabcpp/experimental/testing/protocol.h"
#include "emlabcpp/experimental/testing/reactor_interface_adapter.h"
#include "emlabcpp/pmr/stack_resource.h"
#include "emlabcpp/protocol/endpoint.h"
#include "emlabcpp/result.h"

namespace emlabcpp::testing
{

class reactor
{
        protocol::channel_type channel_;

        std::string_view       suite_name_;
        const std::string_view suite_date_ = __DATE__ " " __TIME__;

        empty_node                        root_node_;
        pmr::stack_resource< 1024 + 512 > mem_;

        reactor_interface_adapter iface_;

        std::optional< executor > opt_exec_;
        bool                      boot_msg_fired_ = false;

public:
        explicit reactor(
            const protocol::channel_type chann,
            const std::string_view       suite_name,
            reactor_transmit_callback    tb )
          : channel_( chann )
          , suite_name_( suite_name )
          , iface_( chann, std::move( tb ) )
        {
        }

        reactor( const reactor& )            = delete;
        reactor( reactor&& )                 = delete;
        reactor& operator=( const reactor& ) = delete;
        reactor& operator=( reactor&& )      = delete;

        [[nodiscard]] constexpr protocol::channel_type get_channel() const
        {
                return channel_;
        }

        outcome on_msg( std::span< const std::byte > buffer );
        outcome on_msg( const controller_reactor_variant& var );

        void register_test( linked_list_node_base< test_interface >& test )
        {
                root_node_.link_as_next( test );
        }

        void tick();

private:
        outcome handle_message( get_property< msgid::SUITE_NAME > );
        outcome handle_message( get_property< msgid::SUITE_DATE > );
        outcome handle_message( get_property< msgid::COUNT > );
        outcome handle_message( get_test_name_request );
        outcome handle_message( exec_request );
};

}  // namespace emlabcpp::testing
