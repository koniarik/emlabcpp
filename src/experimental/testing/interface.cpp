#include "emlabcpp/experimental/testing/interface.h"

#include "emlabcpp/experimental/testing/reactor.h"

namespace emlabcpp::testing
{

test_interface::test_interface( const name_buffer& name )
  : name( name )
{
}

test_interface::test_interface( const std::string_view name )
  : test_interface( name_to_buffer( name ) )
{
}

test_coroutine test_interface::setup( pmr::memory_resource&, record& )
{
        co_return;
}
test_coroutine test_interface::teardown( pmr::memory_resource&, record& )
{
        co_return;
}

}  // namespace emlabcpp::testing
