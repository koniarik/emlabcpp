#include "emlabcpp/experimental/testing/interface.h"

#include "emlabcpp/experimental/testing/reactor.h"

namespace emlabcpp::testing
{

test_interface::test_interface( reactor& rec, const name_buffer& name )
  : name( name )
{
        test_interface& other = rec.get_first_dummy_test();
        other.push_after( this );
}

test_interface::test_interface( reactor& rec, std::string_view name )
  : test_interface( rec, name_to_buffer( name ) )
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

