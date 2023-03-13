
#include "emlabcpp/experimental/geom/simplex.h"

#include "emlabcpp/experimental/geom/json.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

TEST( SimplexTest, volume2d )
{
        std::array< point< 2 >, 3 > data{
            point< 2 >( 0, 1 ), point< 2 >( 1, 0 ), point< 2 >( 0, 0 ) };

        simplex< point< 2 >, 2 > sim( data );

        ASSERT_EQ( volume_of( sim ), 0.5f );
}
TEST( SimplexTest, volume3d )
{
        std::array< point< 3 >, 4 > data{
            point< 3 >( 0, 0, 1 ),
            point< 3 >( 1, 0, 0 ),
            point< 3 >( 0, 0, 0 ),
            point< 3 >{ 0, 1, 0 } };

        simplex< point< 3 >, 3 > sim( data );

        ASSERT_NEAR( volume_of( sim ), 0.166667f, 0.00001f );
}

TEST( SimplexTest, json )
{
        std::array< point< 3 >, 4 > data{
            point< 3 >( 0, 0, 1 ),
            point< 3 >( 1, 0, 0 ),
            point< 3 >( 0, 0, 0 ),
            point< 3 >{ 0, 1, 0 } };

        using simplex_type = simplex< point< 3 >, 3 >;

        simplex_type sim( data );

        nlohmann::json simplex_j = nlohmann::json( sim );
        ASSERT_EQ( simplex_j.get< simplex_type >(), sim );
}
