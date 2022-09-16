#include "emlabcpp/experimental/function_view.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

using basic_view   = function_view< void( int ) >;
using return_view  = function_view< std::string( int ) >;
using complex_view = function_view< float( int, std::string ) >;

// NOLINTNEXTLINE
TEST( FunctionView, call_view_pointer )
{
        auto       lb = []( int ) -> void {};
        basic_view bf{ lb };
        bf( 42 );

        auto lr = []( int val ) -> std::string {
                return std::to_string( val );
        };
        return_view rf{ lr };
        std::string sub_res = rf( 42 );
        EXPECT_EQ( sub_res, "42" );

        auto lc = []( int val, std::string sval ) -> float {
                return static_cast< float >( val * std::stoi( sval ) );
        };
        static_assert( with_signature< decltype( lc ), float( int, std::string ) > );
        complex_view cf{ lc };
        float        val = cf( 42, "2" );
        EXPECT_EQ( val, 84 );
}

// NOLINTNEXTLINE
TEST( FunctionView, call_callable )
{
        int  test_value = 0;
        auto lf         = [&]( int val ) -> void {
                test_value = val;
        };
        basic_view bf{ lf };
        bf( 42 );
        EXPECT_EQ( test_value, 42 );
        bf( 666 );
        EXPECT_EQ( test_value, 666 );

        test_value = 0;
        auto lr    = [&]( int val ) -> std::string {
                test_value = val;
                return std::to_string( val );
        };
        return_view rf{ lr };
        std::string test_sres = rf( 42 );
        EXPECT_EQ( test_value, 42 );
        EXPECT_EQ( test_sres, "42" );
        test_sres = rf( 666 );
        EXPECT_EQ( test_value, 666 );
        EXPECT_EQ( test_sres, "666" );

        test_value = 0;
        auto lc    = [&]( int val, std::string sval ) -> float {
                test_value = val;
                return static_cast< float >( std::stoi( sval ) );
        };
        complex_view cf{ lc };
        float        test_fres = cf( 42, "12" );
        EXPECT_EQ( test_value, 42 );
        EXPECT_EQ( test_fres, 12.f );
        test_fres = cf( 666, "13" );
        EXPECT_EQ( test_value, 666 );
        EXPECT_EQ( test_fres, 13.f );
}
