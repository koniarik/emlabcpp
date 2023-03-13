
#include "emlabcpp/experimental/geom/quaternion.h"

#include "emlabcpp/experimental/geom/point.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

TEST( Quaternion, global_constants )
{
        quaternion v{ 0, 0, 0, 1.f };
        EXPECT_EQ( neutral_quat, v );
}

TEST( Quaternion, rotation )
{
        point< 3 > p{ 1.f, 0.f, 0.f };

        point< 3 > res_p = rotate( p, quaternion( y_axis, *pi / 4.f ) );
        point< 3 > desired{ 0.70710678118f, 0, -0.70710678118f };
        EXPECT_TRUE( almost_equal( res_p, desired ) );
}
