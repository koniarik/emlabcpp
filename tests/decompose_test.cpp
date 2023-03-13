///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
/// associated documentation files (the "Software"), to deal in the Software without restriction,
/// including without limitation the rights to use, copy, modify, merge, publish, distribute,
/// sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or substantial
/// portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
/// NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
/// OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
/// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

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

struct tricky_opt
{
        int                          i;
        std::optional< std::string > opt_string;
};

// NOLINTNEXTLINE
TEST( Decompose, tricky )
{
        tricky_opt                                        to{ 42, "test" };
        std::tuple< int&, std::optional< std::string >& > tov = decompose( to );
        EXPECT_EQ( std::get< 0 >( tov ), to.i );
        EXPECT_EQ( std::get< 1 >( tov ), to.opt_string );
}
