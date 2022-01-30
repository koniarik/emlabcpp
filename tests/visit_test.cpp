
#include "emlabcpp/visit.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

TEST( visit, vis )
{
        std::variant< int, std::string > var{ 42 };
        bool                             fired = false;

        visit(
            [&]< typename T >( T ) {
                    bool is_int = std::is_same_v< T, int >;
                    EXPECT_TRUE( is_int );

                    fired = true;
            },
            var );
        EXPECT_TRUE( fired );
        fired = false;

        var = std::string{ "test" };

        visit(
            [&]< typename T >( T ) {
                    bool is_string = std::is_same_v< T, std::string >;
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
