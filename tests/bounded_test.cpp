/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
///

#include "emlabcpp/bounded.h"

#include "./util/util.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

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
        EXPECT_EQ( *test_bounded::make( 0 ).value(), 0 );
        EXPECT_EQ( *test_bounded::make( 1 ).value(), 1 );
        EXPECT_EQ( *test_bounded::make( -1 ).value(), -1 );

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

        EXPECT_EQ( *val, *test_bounded::min() );

        val.rotate_left( 1 );

        EXPECT_EQ( *val, *test_bounded::max() );
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
        // auto v = bounded_constant< 42 >;

        // TODO: fix
        // EXPECT_EQ( to_string( v ), "42" );
}

}  // namespace emlabcpp
