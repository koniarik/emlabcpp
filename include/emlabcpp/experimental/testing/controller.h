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

namespace emlabcpp
{

class testing_controller_interface_adapter;

class testing_controller
{
public:
        static std::optional< testing_controller >
        make( testing_controller_interface& iface, pool_interface* );

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

        [[nodiscard]] const pool_map< testing_test_id, test_info >& get_tests() const
        {
                return tests_;
        }

        void start_test( testing_test_id tid, testing_controller_interface& iface );

        void tick( testing_controller_interface& iface );

private:
        pool_map< testing_test_id, test_info > tests_;
        testing_name_buffer                    name_;
        testing_name_buffer                    date_;
        std::optional< testing_result >        context_;
        testing_run_id                         rid_ = 0;
        pool_interface*                        mem_pool_;

        testing_controller(
            testing_name_buffer                    name,
            testing_name_buffer                    date,
            pool_map< testing_test_id, test_info > tests,
            pool_interface*                        mem_pool )
          : tests_( std::move( tests ) )
          , name_( std::move( name ) )
          , date_( std::move( date ) )
          , mem_pool_( mem_pool )
        {
        }

        void
        handle_message( tag< TESTING_COUNT >, auto, testing_controller_interface_adapter& iface );
        void
        handle_message( tag< TESTING_NAME >, auto, testing_controller_interface_adapter& iface );
        void handle_message(
            tag< TESTING_PARAM_VALUE >,
            testing_run_id                        rid,
            testing_node_id                       nid,
            testing_controller_interface_adapter& iface );
        void handle_message(
            tag< TESTING_PARAM_CHILD >,
            testing_run_id                                       rid,
            testing_node_id                                      nid,
            const std::variant< testing_key, testing_child_id >& chid,
            testing_controller_interface_adapter&                iface );
        void handle_message(
            tag< TESTING_PARAM_CHILD_COUNT >,
            testing_run_id                        rid,
            testing_node_id                       nid,
            testing_controller_interface_adapter& iface );
        void handle_message(
            tag< TESTING_PARAM_KEY >,
            testing_run_id                        rid,
            testing_node_id                       nid,
            testing_child_id                      chid,
            testing_controller_interface_adapter& iface );
        void handle_message(
            tag< TESTING_PARAM_TYPE >,
            testing_run_id                        rid,
            testing_node_id                       nid,
            testing_controller_interface_adapter& iface );

        void handle_message(
            tag< TESTING_COLLECT >,
            testing_run_id                      rid,
            testing_node_id                     parent,
            const std::optional< testing_key >& opt_key,
            const testing_collect_arg&          val,
            testing_controller_interface_adapter& );
        void handle_message(
            tag< TESTING_FINISHED >,
            auto,
            testing_controller_interface_adapter& iface );
        void handle_message( tag< TESTING_ERROR >, auto, testing_controller_interface_adapter& );
        void handle_message( tag< TESTING_FAILURE >, auto, testing_controller_interface_adapter& );
        void handle_message(
            tag< TESTING_SUITE_NAME >,
            auto,
            testing_controller_interface_adapter& iface );
        void handle_message(
            tag< TESTING_SUITE_DATE >,
            auto,
            testing_controller_interface_adapter& iface );
        void handle_message(
            tag< TESTING_INTERNAL_ERROR >,
            testing_reactor_error_variant         err,
            testing_controller_interface_adapter& iface );
        void handle_message(
            tag< TESTING_PROTOCOL_ERROR >,
            protocol::error_record                rec,
            testing_controller_interface_adapter& iface );
};

}  // namespace emlabcpp
