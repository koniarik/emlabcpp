#include "emlabcpp/experimental/testing/interface.h"

#include "emlabcpp/experimental/testing/reactor.h"

#include <utility>

namespace emlabcpp::testing
{

test_interface::test_interface( name_buffer name )
  : name( std::move( name ) )
{
}

test_interface::test_interface( const std::string_view name )
  : test_interface( name_to_buffer( name ) )
{
}

test_interface::test_interface( reactor& rec, name_buffer name )
  : name( std::move( name ) )
{
        rec.register_test( *this );
}

test_interface::test_interface( reactor& rec, const std::string_view name )
  : test_interface( name_to_buffer( name ) )
{
        rec.register_test( *this );
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
