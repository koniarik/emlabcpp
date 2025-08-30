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

#include "emlabcpp/experimental/function_view.h"

#include "emlabcpp/concepts.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

using basic_view   = function_view< void( int ) >;
using return_view  = function_view< std::string( int ) >;
using complex_view = function_view< float( int, std::string const& ) >;

// NOLINTNEXTLINE
TEST( FunctionView, call_view_pointer )
{
        auto             lb = []( int ) -> void {};
        basic_view const bf{ lb };
        bf( 42 );

        auto lr = []( int val ) -> std::string {
                return std::to_string( val );
        };
        return_view const rf{ lr };
        std::string const sub_res = rf( 42 );
        EXPECT_EQ( sub_res, "42" );

        auto lc = []( int val, std::string const& sval ) -> float {
                return static_cast< float >( val * std::stoi( sval ) );
        };
        static_assert( with_signature< decltype( lc ), float( int, std::string const& ) > );
        complex_view const cf{ lc };
        float const        val = cf( 42, "2" );
        EXPECT_EQ( val, 84 );
}

// NOLINTNEXTLINE
TEST( FunctionView, call_callable )
{
        int  test_value = 0;
        auto lf         = [&]( int val ) -> void {
                test_value = val;
        };
        basic_view const bf{ lf };
        bf( 42 );
        EXPECT_EQ( test_value, 42 );
        bf( 666 );
        EXPECT_EQ( test_value, 666 );

        test_value = 0;
        auto lr    = [&]( int val ) -> std::string {
                test_value = val;
                return std::to_string( val );
        };
        return_view const rf{ lr };
        std::string       test_sres = rf( 42 );
        EXPECT_EQ( test_value, 42 );
        EXPECT_EQ( test_sres, "42" );
        test_sres = rf( 666 );
        EXPECT_EQ( test_value, 666 );
        EXPECT_EQ( test_sres, "666" );

        test_value = 0;
        auto lc    = [&]( int val, std::string const& sval ) -> float {
                test_value = val;
                return static_cast< float >( std::stoi( sval ) );
        };
        complex_view const cf{ lc };
        float              test_fres = cf( 42, "12" );
        EXPECT_EQ( test_value, 42 );
        EXPECT_EQ( test_fres, 12.f );
        test_fres = cf( 666, "13" );
        EXPECT_EQ( test_value, 666 );
        EXPECT_EQ( test_fres, 13.f );
}

}  // namespace emlabcpp
