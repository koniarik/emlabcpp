#include "emlabcpp/experimental/static_function.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

using basic_function   = static_function< void( int ), 42 >;
using return_function  = static_function< std::string( int ), 42 >;
using complex_function = static_function< float( int, std::string ), 42 >;

// NOLINTNEXTLINE
TEST( StaticFunction, empty )
{
        basic_function bf;
        EXPECT_FALSE( bf );

        return_function rf;
        EXPECT_FALSE( rf );

        complex_function cf;
        EXPECT_FALSE( cf );
}

// NOLINTNEXTLINE
TEST( StaticFunction, store_function_pointer )
{
        basic_function bf = []( int ) -> void {};
        EXPECT_TRUE( bf );
        bf = nullptr;
        EXPECT_FALSE( bf );

        return_function rf = []( int ) -> std::string {
                return "";
        };
        EXPECT_TRUE( rf );
        rf = nullptr;
        EXPECT_FALSE( rf );

        complex_function cf = []( int, std::string ) -> float {
                return 0.f;
        };
        EXPECT_TRUE( cf );
        cf = nullptr;
        EXPECT_FALSE( cf );
}

// NOLINTNEXTLINE
TEST( StaticFunction, call_function_pointer )
{
        basic_function bf = []( int ) -> void {};
        EXPECT_TRUE( bf );
        bf( 42 );

        return_function rf = []( int val ) -> std::string {
                return std::to_string( val );
        };
        EXPECT_TRUE( rf );
        std::string sub_res = rf( 42 );
        EXPECT_EQ( sub_res, "42" );

        complex_function cf = []( int val, std::string sval ) -> float {
                return static_cast< float >( val * std::stoi( sval ) );
        };
        EXPECT_TRUE( cf );
        float val = cf( 42, "2" );
        EXPECT_EQ( val, 84 );
}

// NOLINTNEXTLINE
TEST( StaticFunction, call_callable )
{
        int            test_value = 0;
        basic_function bf         = [&]( int val ) -> void {
                test_value = val;
        };
        EXPECT_TRUE( bf );
        bf( 42 );
        EXPECT_EQ( test_value, 42 );
        bf( 666 );
        EXPECT_EQ( test_value, 666 );

        test_value         = 0;
        return_function rf = [&]( int val ) -> std::string {
                test_value = val;
                return std::to_string( val );
        };
        EXPECT_TRUE( rf );
        std::string test_sres = rf( 42 );
        EXPECT_EQ( test_value, 42 );
        EXPECT_EQ( test_sres, "42" );
        test_sres = rf( 666 );
        EXPECT_EQ( test_value, 666 );
        EXPECT_EQ( test_sres, "666" );

        test_value          = 0;
        complex_function cf = [&]( int val, std::string sval ) -> float {
                test_value = val;
                return static_cast< float >( std::stoi( sval ) );
        };
        EXPECT_TRUE( cf );
        float test_fres = cf( 42, "12" );
        EXPECT_EQ( test_value, 42 );
        EXPECT_EQ( test_fres, 12.f );
        test_fres = cf( 666, "13" );
        EXPECT_EQ( test_value, 666 );
        EXPECT_EQ( test_fres, 13.f );
}
