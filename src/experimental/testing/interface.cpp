#include "emlabcpp/experimental/testing/interface.h"

#include <utility>

namespace emlabcpp::testing
{

test_coroutine test_interface::setup( pmr::memory_resource&, record& )
{
        co_return;
}
test_coroutine test_interface::teardown( pmr::memory_resource&, record& )
{
        co_return;
}

}  // namespace emlabcpp::testing
