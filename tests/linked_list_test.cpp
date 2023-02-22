#include "emlabcpp/experimental/linked_list.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

using test_node = linked_list_node< int >;

TEST( LinkedList, base )
{
        test_node n1{ 1 };
        test_node n2{ 2 };
        n1.link_as_next( n2 );

        EXPECT_EQ( n1.get_next(), &n2 );

        test_node n3 = std::move( n2 );

        EXPECT_EQ( n1.get_next(), &n3 );
}

}  // namespace emlabcpp
