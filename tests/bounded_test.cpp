
#include "emlabcpp/bounded.h"

#include "util.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

using test_bounded = bounded< int, -1, 1 >;

// NOLINTNEXTLINE
TEST( Bounded, get )
{
        EXPECT_EQ( *test_bounded::get< 0 >(), 0 );
        EXPECT_EQ( *test_bounded::get< 1 >(), 1 );
        EXPECT_EQ( *test_bounded::get< -1 >(), -1 );
}

// NOLINTNEXTLINE
TEST( Bounded, make )
{
        EXPECT_EQ( **test_bounded::make( 0 ), 0 );
        EXPECT_EQ( **test_bounded::make( 1 ), 1 );
        EXPECT_EQ( **test_bounded::make( -1 ), -1 );

        EXPECT_FALSE( test_bounded::make( 2 ).has_value() );
        EXPECT_FALSE( test_bounded::make( -2 ).has_value() );
}

// NOLINTNEXTLINE
TEST( Bounded, minmax )
{
        EXPECT_EQ( *test_bounded::min(), -1 );
        EXPECT_EQ( *test_bounded::max(), 1 );
}

// NOLINTNEXTLINE
TEST( Bounded, type_conversion )
{
        EXPECT_EQ( int( test_bounded::min() ), -1 );
        EXPECT_EQ( int( test_bounded::max() ), 1 );
}

// NOLINTNEXTLINE
TEST( Bounded, rotate )
{
        auto val = test_bounded::max();

        val.rotate_right( 1 );

        EXPECT_EQ( val, test_bounded::min() );

        val.rotate_left( 1 );

        EXPECT_EQ( val, test_bounded::max() );
}

// NOLINTNEXTLINE
TEST( Bounded, cmp )
{
        EXPECT_TRUE( test_bounded::max() <=> 1 == 0 );
        EXPECT_FALSE( test_bounded::max() <=> 0 == 0 );
}

// NOLINTNEXTLINE
TEST( Bounded, constant )
{
        auto v = bounded_constant< 42 >;

        EXPECT_TRUE( decltype( v )::max() <=> 42u == 0 );
        EXPECT_TRUE( decltype( v )::min() <=> 42u == 0 );
}

static_assert( !bounded_derived< int > );
static_assert( !bounded_derived< std::string > );
static_assert( bounded_derived< test_bounded > );

struct bounded_derived_test : test_bounded
{
};
static_assert( bounded_derived< bounded_derived_test > );

// NOLINTNEXTLINE
TEST( Bounded, ostream )
{
        auto v = bounded_constant< 42 >;

        EXPECT_EQ( to_string( v ), "42" );
}
