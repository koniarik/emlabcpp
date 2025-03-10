/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
///

#include "util/test.h"

#include "emlabcpp/experimental/testing/executor.h"
#include "emlabcpp/pmr/new_delete_resource.h"
#include "util/util.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

void executor_test_run( auto cb, testing::test_status s )
{
        testing::test_callable tcb{ "tcb", std::move( cb ) };
        testing::executor      exec{
            static_cast< testing::test_id >( 0 ), pmr::new_delete_resource(), tcb };

        while ( !exec.finished() )
                exec.tick();

        EXPECT_EQ( exec.status(), s );
}

TEST( executor, empty_run )
{
        executor_test_run(
            [&]( pmr::memory_resource& ) -> testing::coroutine< void > {
                    co_return;
            },
            testing::test_status::SUCCESS );
}

TEST( executor, expect_pass )
{
        executor_test_run(
            [&]( pmr::memory_resource& ) -> testing::coroutine< void > {
                    co_await testing::expect( true );
            },
            testing::test_status::SUCCESS );
}

TEST( executor, expect_failure )
{
        executor_test_run(
            [&]( pmr::memory_resource& ) -> testing::coroutine< void > {
                    co_await testing::expect( false );
            },
            testing::test_status::FAILED );
}

TEST( executor, skip )
{
        executor_test_run(
            [&]( pmr::memory_resource& ) -> testing::coroutine< void > {
                    co_await testing::skip();
            },
            testing::test_status::SKIPPED );
}

}  // namespace emlabcpp
