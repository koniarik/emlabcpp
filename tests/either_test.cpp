// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 
//
//  Copyright © 2022 Jan Veverak Koniarik
//  This file is part of project: emlabcpp
//
#include "emlabcpp/either.h"

#include <gtest/gtest.h>

using namespace emlabcpp;
using test_either = either< std::string, int >;

static_assert( std::regular< test_either > );

// NOLINTNEXTLINE
TEST( Either, init )
{
        test_either eth{ 0 };
        test_either eth2{ std::string{ "wololo" } };

        either< int, int > val{ 42 };
}

// NOLINTNEXTLINE
TEST( Either, assignment )
{
        test_either eth{ 0 };

        eth = std::string{ "wololo" };
        eth = 0;

        either< int, int > symm{ 0 };

        symm = 42;
        symm = 22;
}

// NOLINTNEXTLINE
TEST( Either, convert )
{
        test_either eth{ 0 };

        EXPECT_TRUE( eth.convert_left( [&]( const std::string& ) {
                                return 0;
                        } )
                         .is_left() );
        EXPECT_FALSE( eth.convert_right( [&]( int ) {
                                 return "wololo";
                         } )
                          .is_left() );
}

// NOLINTNEXTLINE
TEST( Either, match )
{
        test_either eth{ 0 };

        eth.match(
            [&]( const std::string& ) {
                    FAIL();
            },
            [&]( int val ) {
                    EXPECT_EQ( val, 0 );
            } );
}

// NOLINTNEXTLINE
TEST( Either, assemble_left_collect_right )
{
        using test_either_2 = either< float, int >;

        either< std::tuple< std::string, std::string, float >, static_vector< int, 3 > >
            assemble_either = assemble_left_collect_right(
                test_either{ "wolololo" }, test_either{ "nope" }, test_either_2{ 0.f } );

        EXPECT_TRUE( assemble_either.is_left() );

        auto assemble_either_2 =
            assemble_left_collect_right( test_either{ "wololo" }, test_either{ 666 } );

        EXPECT_FALSE( assemble_either_2.is_left() );
}
