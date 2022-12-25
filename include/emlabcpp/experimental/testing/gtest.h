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
#include "emlabcpp/experimental/testing/controller.h"
#include "emlabcpp/experimental/testing/json.h"
#include "emlabcpp/pmr/new_delete_resource.h"

#ifdef EMLABCPP_USE_GTEST

#pragma once

#include <gtest/gtest.h>

namespace emlabcpp::testing
{

inline ::testing::AssertionResult gtest_predicate( const char*, const test_result& tres )
{

        ::testing::AssertionResult res = ::testing::AssertionSuccess();
        if ( tres.failed ) {
                res = ::testing::AssertionFailure() << "Test produced a failure, stopping";
        } else if ( tres.errored ) {
                res = ::testing::AssertionFailure() << "Test errored";
        }

        return res;
}

class gtest : public ::testing::Test
{
        test_id     tid_;
        controller& cont_;

public:
        gtest( controller& cont, test_id tid )
          : tid_( tid )
          , cont_( cont )
        {
        }

        void TestBody() final
        {
                cont_.start_test( tid_ );
                while ( cont_.is_test_running() ) {
                        cont_.tick();
                }
        }
};

void register_gtests( controller& cont )
{
        std::string suite_name = std::string{ cont.suite_name() };
        for ( auto [tid, tinfo] : cont.get_tests() ) {
                std::string name{ tinfo.name.begin(), tinfo.name.end() };
                test_id     test_id = tid;
                ::testing::RegisterTest(
                    suite_name.c_str(),
                    name.c_str(),
                    nullptr,
                    nullptr,
                    __FILE__,
                    __LINE__,
                    [&cont, test_id] {
                            return new gtest( cont, test_id );
                    } );
        }
}

}  // namespace emlabcpp::testing

#endif
