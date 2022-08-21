#include "emlabcpp/experimental/decompose.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

struct one_item
{
        int i;
};
struct two_item
{
        int         i;
        std::string s;
};
struct three_item
{
        int         i;
        std::string s;
        float       f;
};

// NOLINTNEXTLINE
TEST( Decompose, decompose )
{
        one_item v1{ 42 };

        std::tuple< int& > t1 = decompose( v1 );
        EXPECT_EQ( std::get< 0 >( t1 ), v1.i );

        std::tuple< int > t1b = decompose( one_item{ 1 } );
        EXPECT_EQ( std::get< 0 >( t1b ), 1 );

        two_item v2{ 42, "test" };

        std::tuple< int&, std::string& > t2 = decompose( v2 );
        EXPECT_EQ( std::get< 0 >( t2 ), v2.i );
        EXPECT_EQ( std::get< 1 >( t2 ), v2.s );

        three_item v3{ 42, "test", 3.141592f };

        std::tuple< int&, std::string&, float& > t3 = decompose( v3 );
        EXPECT_EQ( std::get< 0 >( t3 ), v3.i );
        EXPECT_EQ( std::get< 1 >( t3 ), v3.s );
        EXPECT_EQ( std::get< 2 >( t3 ), v3.f );
}
