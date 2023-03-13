

#include <gtest/gtest.h>

#pragma once

namespace emlabcpp
{
template < typename IC >
class PointTest : public ::testing::Test
{
public:
        static constexpr unsigned n = IC::value;

        void SetUp() override
        {
                zero_p_    = point< n >::make_filled_with( 0.f );
                unit_p_    = point< n >::make_filled_with( 1.f );
                dim_p_     = point< n >::make_filled_with( float( n ) );
                neg_dim_p_ = point< n >::make_filled_with( -float( n ) );
        }

        point< n > zero_p_;
        point< n > unit_p_;
        point< n > dim_p_;
        point< n > neg_dim_p_;
};

using test_types = ::testing::Types<
    std::integral_constant< unsigned, 2 >,
    std::integral_constant< unsigned, 3 >,
    std::integral_constant< unsigned, 4 >,
    std::integral_constant< unsigned, 8 > >;

TYPED_TEST_CASE( PointTest, test_types );

}  // namespace emlabcpp
