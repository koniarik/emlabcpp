#include "emlabcpp/either.h"

#include <gtest/gtest.h>

using namespace emlabcpp;
using test_either = either< std::string, int >;

TEST( Either, init )
{
        test_either eth{ 0 };
        test_either eth2{ std::string{ "wololo" } };

        either< int, int > val{ 42 };
}

TEST( Either, assignment )
{
        test_either eth{ 0 };

        eth = std::string{ "wololo" };
        eth = 0;

        either< int, int > symm{ 0 };

        symm = 42;
        symm = 22;
}

TEST( Either, convert )
{
        test_either eth{ 0 };

        EXPECT_TRUE( eth.convert_left( [&]( std::string ) {
                                return 0;
                        } )
                         .is_left() );
        EXPECT_FALSE( eth.convert_right( [&]( int ) {
                                 return "wololo";
                         } )
                          .is_left() );
}

TEST( Either, match )
{
        test_either eth{ 0 };

        eth.match(
            [&]( std::string ) {
                    FAIL();
            },
            [&]( int val ) {
                    EXPECT_EQ( val, 0 );
            } );
}

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
