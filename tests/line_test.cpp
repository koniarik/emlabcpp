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

#include "emlabcpp/experimental/geom/line.h"

#include "util/point_test.h"

#include <cmath>
#include <gtest/gtest.h>

namespace emlabcpp
{

TYPED_TEST( PointTest, line_point_distance )
{
        static constexpr unsigned n = TypeParam::value;
        line< n > const           test_line{ this->zero_p_, this->unit_p_ };

        ASSERT_NEAR( distance_of( test_line, this->unit_p_ ), 0, 1e-6 );
        ASSERT_NEAR( distance_of( test_line, this->zero_p_ ), 0, 1e-6 );
        ASSERT_NEAR( distance_of( test_line, -this->unit_p_ ), length_of( this->unit_p_ ), 1e-6 );
}

TYPED_TEST( PointTest, perpendicular )
{
        static constexpr unsigned n = TypeParam::value;
        point< 2 > const          p{ { n, 0 } };
        point< 2 > const          zero_p{ { 0, 0 } };
        point< 2 > const          unit_p{ { n, n } };

        float const diag_dist = std::sqrt( static_cast< float >( 2 * n * n ) ) / 2.f;

        line< 2 > const test_line{ zero_p, unit_p };
        ASSERT_NEAR( distance_of( test_line, p ), diag_dist, 1e-6 );

        line< 2 > const reversed_line{ unit_p, zero_p };
        ASSERT_NEAR( distance_of( reversed_line, p ), diag_dist, 1e-6 );

        line< 2 > const diag_line{ point< 2 >{ { 0, n } }, point< 2 >{ { n, 0 } } };
        ASSERT_NEAR( distance_of( diag_line, zero_p ), diag_dist, 1e-6 );
        ASSERT_NEAR( distance_of( diag_line, unit_p ), diag_dist, 1e-6 );
}

}  // namespace emlabcpp
