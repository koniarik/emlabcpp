///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
/// and associated documentation files (the "Software"), to deal in the Software without
/// restriction, including without limitation the rights to use, copy, modify, merge, publish,
/// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or
/// substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
/// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
/// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#include "emlabcpp/experimental/geom/pose.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

float sum_distance( pose_distance pd )
{
        return pd.dist + pd.angle_dist;
}

TEST( Pose, distance )
{
        pose neutral;
        pose pos_offseted{ point< 3 >( 10, 0, 0 ) };
        pose angle_offseted{ quaternion( z_axis, 1.f ) };

        ASSERT_NEAR( sum_distance( distance_of( neutral, pos_offseted ) ), 10., 1e-6 );
        ASSERT_NEAR( sum_distance( distance_of( neutral, angle_offseted ) ), 1., 1e-6 );
}

TEST( Pose, interp )
{
        pose neutral;
        pose pos_offseted{ point< 3 >( 10, 0, 0 ) };
        pose angle_offseted{ quaternion( z_axis, 1.f ) };

        pose offseted{ point< 3 >( 10, 0, 0 ), quaternion( z_axis, 1.f ) };

        pose interpolated = lin_interp( neutral, pos_offseted, 0.5 );
        pose interpolated_exp{ point< 3 >( 5, 0, 0 ) };

        ASSERT_TRUE( almost_equal( interpolated, interpolated_exp, default_epsilon * 2 ) );

        interpolated     = lin_interp( neutral, angle_offseted, 0.5 );
        interpolated_exp = pose{ quaternion( z_axis, 0.5f ) };

        ASSERT_TRUE( almost_equal( interpolated, interpolated_exp, default_epsilon * 2 ) );

        pose half_offseted{ point< 3 >( 5, 0, 0 ), quaternion( z_axis, 0.5f ) };
        ASSERT_TRUE( almost_equal(
            lin_interp( neutral, offseted, 0.5 ), half_offseted, default_epsilon * 2 ) );
}
