

#include "emlabcpp/match.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

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
            [&]( std::string ) {
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
            [&]( std::string ) {
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
            [&]( std::string ) {
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
            [&]( std::string ) {
                    SUCCEED();
                    fired = true;
            } );

        EXPECT_TRUE( fired );
}
