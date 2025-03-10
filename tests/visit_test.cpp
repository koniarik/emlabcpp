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

#include "emlabcpp/visit.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

TEST( visit, vis )
{
        std::variant< int, std::string > var{ 42 };
        bool                             fired = false;

        visit(
            [&]< typename T >( T ) {
                    const bool is_int = std::is_same_v< T, int >;
                    EXPECT_TRUE( is_int );

                    fired = true;
            },
            var );
        EXPECT_TRUE( fired );
        fired = false;

        var = std::string{ "test" };

        visit(
            [&]< typename T >( T ) {
                    const bool is_string = std::is_same_v< T, std::string >;
                    EXPECT_TRUE( is_string );

                    fired = true;
            },
            var );
        EXPECT_TRUE( fired );
}

TEST( visit, vis_apply )
{
        std::variant< std::tuple< int, int >, std::tuple< std::string > > var =
            std::make_tuple( 42, 666 );
        bool fired = false;

        apply_on_visit(
            [&]< typename... Ts >( Ts... ) {
                    EXPECT_EQ( sizeof...( Ts ), 2 );
                    fired = true;
            },
            var );

        EXPECT_TRUE( fired );

        var   = std::make_tuple( std::string{ "test" } );
        fired = false;

        apply_on_visit(
            [&]< typename... Ts >( Ts... ) {
                    EXPECT_EQ( sizeof...( Ts ), 1 );
                    fired = true;
            },
            var );

        EXPECT_TRUE( fired );
}

}  // namespace emlabcpp
