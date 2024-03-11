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

#include "emlabcpp/experimental/testing/base.h"
#include "emlabcpp/experimental/testing/controller_interface_adapter.h"
#include "emlabcpp/experimental/testing/coroutine.h"
#include "emlabcpp/pmr/aliases.h"
#include "emlabcpp/result.h"

namespace emlabcpp::testing
{

class controller
{
        struct initializing_state
        {
                coroutine< void > coro{};
        };

        struct test_running_state
        {
                test_result context;
        };

        struct idle_state
        {
        };

        using states = std::variant< initializing_state, test_running_state, idle_state >;

public:
        controller(
            const protocol::channel_type channel,
            pmr::memory_resource&        mem_res,
            controller_interface&        iface,
            controller_transmit_callback send_cb )
          : channel_( channel )
          , mem_res_( mem_res )
          , iface_( channel_, iface, std::move( send_cb ) )
          , tests_( mem_res )
        {
                auto* const i_state_ptr = std::get_if< initializing_state >( &state_ );
                EMLABCPP_ASSERT( i_state_ptr != nullptr );
                i_state_ptr->coro = initialize( mem_res );
        }

        [[nodiscard]] constexpr protocol::channel_type get_channel() const
        {
                return channel_;
        }

        [[nodiscard]] std::string_view suite_name() const
        {
                return { name_.begin(), name_.end() };
        }

        [[nodiscard]] std::string_view suite_date() const
        {
                return { date_.begin(), date_.end() };
        }

        [[nodiscard]] bool is_initializing() const
        {
                return std::holds_alternative< initializing_state >( state_ );
        }

        [[nodiscard]] bool is_test_running() const
        {
                return std::holds_alternative< test_running_state >( state_ );
        }

        [[nodiscard]] const pmr::map< test_id, name_buffer >& get_tests() const
        {
                return tests_;
        }

        outcome on_msg( const std::span< const std::byte > data );
        outcome on_msg( const reactor_controller_variant& );

        void start_test( const test_id tid );

        void tick();

private:
        coroutine< void > initialize( pmr::memory_resource& mem_res );

        protocol::channel_type channel_;
        states                 state_ = initializing_state{};

        std::reference_wrapper< pmr::memory_resource > mem_res_;
        controller_interface_adapter                   iface_;
        pmr::map< test_id, name_buffer >               tests_;
        name_buffer                                    name_;
        name_buffer                                    date_;
        run_id                                         rid_ = 0;
};

}  // namespace emlabcpp::testing
