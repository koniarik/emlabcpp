
#include "emlabcpp/experimental/testing/coroutine.h"
#include "emlabcpp/experimental/testing/interface.h"

#pragma once

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

        testing::test_coroutine setup( pmr::memory_resource&, testing::record& ) override
        {
                setup_count += 1;
                co_return;
        }
        testing::test_coroutine run( pmr::memory_resource&, testing::record& ) override
        {
                run_count += 1;
                co_return;
        }
        testing::test_coroutine teardown( pmr::memory_resource&, testing::record& ) override
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

        testing::test_coroutine setup( pmr::memory_resource&, testing::record& ) override
        {
                co_await std::suspend_always{};
                co_await std::suspend_never{};
                setup_count += 1;
        }
        testing::test_coroutine run( pmr::memory_resource&, testing::record& ) override
        {
                co_await std::suspend_always{};
                co_await std::suspend_never{};
                run_count += 1;
        }
        testing::test_coroutine teardown( pmr::memory_resource&, testing::record& ) override
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
