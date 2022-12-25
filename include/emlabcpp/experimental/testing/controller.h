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
#include "emlabcpp/experimental/testing/controller_interface_adapter.h"
#include "emlabcpp/experimental/testing/coroutine.h"
#include "emlabcpp/pmr/aliases.h"

#pragma once

namespace emlabcpp::testing
{

class controller
{
        struct initializing_state
        {
                test_coroutine coro;
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
            pmr::memory_resource&        mem_res,
            controller_interface&        iface,
            controller_transmit_callback send_cb )
          : mem_res_( mem_res )
          , iface_( iface, send_cb )
          , tests_( mem_res )
        {
                initializing_state* i_state_ptr = std::get_if< initializing_state >( &state_ );
                EMLABCPP_ASSERT( i_state_ptr != nullptr );
                i_state_ptr->coro = initialize( mem_res );
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

        [[nodiscard]] const pmr::map< test_id, test_info >& get_tests() const
        {
                return tests_;
        }

        void on_msg( std::span< const uint8_t > data );
        void on_msg( const reactor_controller_variant& );

        void start_test( test_id tid );

        void tick();

private:
        test_coroutine initialize( pmr::memory_resource& mem_res );

        states state_ = initializing_state{};

        std::reference_wrapper< pmr::memory_resource > mem_res_;
        controller_interface_adapter                   iface_;
        pmr::map< test_id, test_info >                 tests_;
        name_buffer                                    name_;
        name_buffer                                    date_;
        run_id                                         rid_ = 0;
};

}  // namespace emlabcpp::testing
