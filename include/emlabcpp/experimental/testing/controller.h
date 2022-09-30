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
#include "emlabcpp/allocator/pool.h"
#include "emlabcpp/experimental/testing/controller_interface.h"
#include "emlabcpp/iterators/convert.h"
#include "emlabcpp/match.h"

#pragma once

namespace emlabcpp::testing
{

class controller_interface_adapter;

class controller
{
public:
        static std::optional< controller > make( controller_interface& iface, pool_interface* );

        [[nodiscard]] std::string_view suite_name() const
        {
                return { name_.begin(), name_.end() };
        }

        [[nodiscard]] std::string_view suite_date() const
        {
                return { date_.begin(), date_.end() };
        }

        [[nodiscard]] bool has_active_test() const
        {
                return context_.has_value();
        }

        [[nodiscard]] const pool_map< test_id, test_info >& get_tests() const
        {
                return tests_;
        }

        void start_test( test_id tid, controller_interface& iface );

        void tick( controller_interface& iface );

private:
        pool_map< test_id, test_info > tests_;
        name_buffer                    name_;
        name_buffer                    date_;
        std::optional< test_result >   context_;
        run_id                         rid_ = 0;
        pool_interface*                mem_pool_;
        controller_endpoint            ep_;

        controller(
            name_buffer                    name,
            name_buffer                    date,
            pool_map< test_id, test_info > tests,
            pool_interface* const          mem_pool )
          : tests_( std::move( tests ) )
          , name_( std::move( name ) )
          , date_( std::move( date ) )
          , mem_pool_( mem_pool )
        {
        }
};

}  // namespace emlabcpp::testing
