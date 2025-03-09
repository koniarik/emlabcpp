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

#include "emlabcpp/experimental/testing/coroutine.h"
#include "emlabcpp/experimental/testing/interface.h"

namespace emlabcpp
{

class simple_test_fixture : public testing::test_interface
{

public:
        using testing::test_interface::test_interface;

        [[nodiscard]] std::string_view get_name() const override
        {
                return "simple wololo";
        }

        testing::coroutine< void > setup( pmr::memory_resource& ) override
        {
                setup_count += 1;
                co_return;
        }

        testing::coroutine< void > run( pmr::memory_resource& ) override
        {
                run_count += 1;
                co_return;
        }

        testing::coroutine< void > teardown( pmr::memory_resource& ) override
        {
                teardown_count += 1;
                co_return;
        }

        std::size_t setup_count    = 0;
        std::size_t run_count      = 0;
        std::size_t teardown_count = 0;
};

class complex_test_fixture : public testing::test_interface
{
public:
        using testing::test_interface::test_interface;

        [[nodiscard]] std::string_view get_name() const override
        {
                return "complex wololo";
        }

        testing::coroutine< void > setup( pmr::memory_resource& ) override
        {
                co_await std::suspend_always{};
                co_await std::suspend_never{};
                setup_count += 1;
        }

        testing::coroutine< void > run( pmr::memory_resource& ) override
        {
                co_await std::suspend_always{};
                co_await std::suspend_never{};
                run_count += 1;
        }

        testing::coroutine< void > teardown( pmr::memory_resource& ) override
        {
                co_await std::suspend_always{};
                co_await std::suspend_never{};
                teardown_count += 1;
        }

        std::size_t setup_count    = 0;
        std::size_t run_count      = 0;
        std::size_t teardown_count = 0;
};

}  // namespace emlabcpp
