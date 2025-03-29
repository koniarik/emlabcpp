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

#include "emlabcpp/experimental/geom/pose.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

float sum_distance( pose_distance pd )
{
        return pd.dist + pd.angle_dist;
}

TEST( Pose, distance )
{
        pose const neutral;
        pose const pos_offseted{ point< 3 >( 10, 0, 0 ) };
        pose const angle_offseted{ quaternion( z_axis, 1.f ) };

        ASSERT_NEAR( sum_distance( distance_of( neutral, pos_offseted ) ), 10., 1e-6 );
        ASSERT_NEAR( sum_distance( distance_of( neutral, angle_offseted ) ), 1., 1e-6 );
}

}  // namespace emlabcpp
