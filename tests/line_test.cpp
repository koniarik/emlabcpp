#include "emlabcpp/experimental/geom/line.h"

#include "point_test.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

TYPED_TEST( PointTest, line_point_distance )
{
        static constexpr unsigned n = TypeParam::value;
        line< n >                 test_line{ this->zero_p_, this->unit_p_ };

        ASSERT_NEAR( distance_of( test_line, this->unit_p_ ), 0, 1e-6 );
        ASSERT_NEAR( distance_of( test_line, this->zero_p_ ), 0, 1e-6 );
        ASSERT_NEAR( distance_of( test_line, -this->unit_p_ ), length_of( this->unit_p_ ), 1e-6 );
}

TYPED_TEST( PointTest, perpendicular )
{
        static constexpr unsigned n = TypeParam::value;
        point< 2 >                p{ { n, 0 } };
        point< 2 >                zero_p{ { 0, 0 } };
        point< 2 >                unit_p{ { n, n } };

        float diag_dist = sqrt( 2 * n * n ) / 2;

        line< 2 > test_line{ zero_p, unit_p };
        ASSERT_NEAR( distance_of( test_line, p ), diag_dist, 1e-6 );

        line< 2 > reversed_line{ unit_p, zero_p };
        ASSERT_NEAR( distance_of( reversed_line, p ), diag_dist, 1e-6 );

        line< 2 > diag_line{ point< 2 >{ { 0, n } }, point< 2 >{ { n, 0 } } };
        ASSERT_NEAR( distance_of( diag_line, zero_p ), diag_dist, 1e-6 );
        ASSERT_NEAR( distance_of( diag_line, unit_p ), diag_dist, 1e-6 );
}
