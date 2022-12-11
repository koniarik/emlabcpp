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
#include "emlabcpp/assert.h"
#include "emlabcpp/defer.h"
#include "emlabcpp/experimental/testing/interface.h"
#include "emlabcpp/experimental/testing/protocol.h"
#include "emlabcpp/experimental/testing/reactor_interface.h"
#include "emlabcpp/pmr/pool_resource.h"
#include "emlabcpp/protocol/packet_handler.h"
#include "emlabcpp/view.h"
#include "emlabcpp/visit.h"

#include <list>

#pragma once

namespace emlabcpp::testing
{

class reactor
{

        struct active_execution
        {
                test_id         tid;
                run_id          rid;
                test_interface* iface_ptr;
        };

        std::string_view suite_name_;
        std::string_view suite_date_ = __DATE__ " " __TIME__;

        empty_test root_test_;

        pmr::pool_resource< 1024, 1 > mem_;
        reactor_endpoint              ep_;

        std::optional< active_execution > active_exec_;

public:
        explicit reactor( const std::string_view suite_name )
          : suite_name_( suite_name )
        {
        }

        reactor( const reactor& )            = delete;
        reactor( reactor&& )                 = delete;
        reactor& operator=( const reactor& ) = delete;
        reactor& operator=( reactor&& )      = delete;

        test_interface& get_first_dummy_test();

        void spin( reactor_interface& comm );

private:
        void handle_message( get_property< SUITE_NAME >, reactor_interface_adapter& ) const;
        void handle_message( get_property< SUITE_DATE >, reactor_interface_adapter& ) const;
        void handle_message( get_property< COUNT >, reactor_interface_adapter& ) const;
        void handle_message( get_test_name_request, reactor_interface_adapter& );
        void handle_message( load_test, reactor_interface_adapter& );
        void handle_message( exec_request, reactor_interface_adapter& );

        void exec_test( reactor_interface_adapter& comm );
};

}  // namespace emlabcpp::testing
