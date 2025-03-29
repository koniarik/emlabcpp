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

#include "emlabcpp/match.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

// NOLINTNEXTLINE
TEST( match, vis )
{

        std::variant< int, std::string > var{ 42 };
        bool                             fired = false;

        match(
            var,
            [&]( int ) {
                    SUCCEED();
                    fired = true;
            },
            [&]( std::string const& ) {
                    FAIL();
            } );

        EXPECT_TRUE( fired );
        fired = false;

        var = std::string{ "test" };

        match(
            var,
            [&]( int ) {
                    FAIL();
            },
            [&]( std::string const& ) {
                    SUCCEED();
                    fired = true;
            } );

        EXPECT_TRUE( fired );
}

// NOLINTNEXTLINE
TEST( match, vis_apply )
{
        std::variant< std::tuple< int, int >, std::tuple< std::string > > var =
            std::make_tuple( 42, 666 );
        bool fired = false;

        apply_on_match(
            var,
            [&]( int, int ) {
                    SUCCEED();
                    fired = true;
            },
            [&]( std::string const& ) {
                    FAIL();
            } );

        EXPECT_TRUE( fired );

        var   = std::make_tuple( std::string{ "test" } );
        fired = false;

        apply_on_match(
            var,
            [&]( int, int ) {
                    FAIL();
            },
            [&]( std::string const& ) {
                    SUCCEED();
                    fired = true;
            } );

        EXPECT_TRUE( fired );
}

TEST( match, referecing )
{
        std::variant< std::vector< int >, std::string > var{ std::string{ "test" } };
        match(
            var,
            [&]( std::vector< int > const& ) {
                    FAIL();
            },
            [&]( std::string& s ) {
                    s = "pololo";
            } );
}

}  // namespace emlabcpp
