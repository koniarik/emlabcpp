#include "emlabcpp/pmr/aliases.h"
#include "emlabcpp/pmr/stack_resource.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

TEST( PMR, stack_resource_list )
{
        pmr::stack_resource< 1024 > stack;

        pmr::list< std::string > l{ stack };

        l.emplace_back( "wololo" );
}

TEST( PMR, stack_resource_list_complex )
{
        pmr::stack_resource< 1024 > stack;

        pmr::list< std::string > l{ stack };

        l.emplace_back( "wololo" );
        l.emplace_back( "tololo" );

        EXPECT_EQ( l.front(), "wololo" );
        EXPECT_EQ( l.back(), "tololo" );

        l.pop_back();

        EXPECT_EQ( l.front(), l.back() );

        l.emplace_back( "kololo" );

        EXPECT_EQ( l.front(), "wololo" );
        EXPECT_EQ( l.back(), "kololo" );
}
