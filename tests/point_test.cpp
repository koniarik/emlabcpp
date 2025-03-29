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

#include "emlabcpp/experimental/geom/point.h"

#include "emlabcpp/experimental/geom/json.h"
#include "util/point_test.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

TEST( Point, dot )
{
        point< 3 > const a{ 0, 1, 0 };
        point< 3 > const b{ -0.5, -0.5, 0 };
        point< 3 > const c{ 1, 1, 0 };

        ASSERT_EQ( dot( a, b ), -0.5f );
        ASSERT_EQ( dot( c, b ), -1.f );
}

TYPED_TEST( PointTest, operators )
{
        static constexpr unsigned n = TypeParam::value;

        ASSERT_EQ( -this->dim_p_, this->neg_dim_p_ );

        point< n > cpy = this->unit_p_;
        cpy += vector_cast( this->unit_p_ );
        auto two_p = point< n >::make_filled_with( 2.f );
        ASSERT_EQ( cpy, two_p );

        cpy = this->unit_p_;
        cpy -= vector_cast( this->unit_p_ );
        ASSERT_EQ( cpy, this->zero_p_ );

        ASSERT_LE( this->zero_p_, this->unit_p_ );

        cpy        = this->zero_p_;
        cpy[n - 1] = 1.f;
        ASSERT_LE( this->zero_p_, cpy );

        ASSERT_EQ( this->unit_p_ * float( n ), this->dim_p_ );

        ASSERT_EQ( this->dim_p_ / float( n ), this->unit_p_ );

        auto bigger = max( this->zero_p_, this->dim_p_ );
        ASSERT_EQ( bigger, this->dim_p_ );
}

TYPED_TEST( PointTest, length )
{
        static constexpr unsigned n = TypeParam::value;
        ASSERT_EQ( length_of( this->zero_p_ ), 0.f );

        ASSERT_NEAR( length_of( this->unit_p_ ), std::sqrt( n ), 1e-6 );
        ASSERT_NEAR( length_of( this->dim_p_ ), std::sqrt( n * n * n ), 1e-6 );
}

TYPED_TEST( PointTest, normalized )
{
        ASSERT_NEAR(
            length_of( normalized( this->dim_p_ ) ),
            length_of( normalized( this->unit_p_ ) ),
            1e-6 );
}

TYPED_TEST( PointTest, json )
{
        static constexpr unsigned n = TypeParam::value;

        nlohmann::json const zero_p_j = nlohmann::json( this->zero_p_ );
        nlohmann::json const unit_p_j = nlohmann::json( this->unit_p_ );
        nlohmann::json const dim_p_j  = nlohmann::json( this->dim_p_ );

        using t = point< n >;

        ASSERT_EQ( zero_p_j.get< t >(), this->zero_p_ );
        ASSERT_EQ( unit_p_j.get< t >(), this->unit_p_ );
        ASSERT_EQ( dim_p_j.get< t >(), this->dim_p_ );
}

// TODO: move this out
TEST( VecTestSimple, crossProduct )
{
        vector< 3 > const a{ 10, 255, 1 };
        vector< 3 > const b{ -22, 0, 4 };

        ASSERT_EQ( dot( cross_product( a, b ), a ), 0.f );
        ASSERT_EQ( dot( cross_product( a, b ), b ), 0.f );
}

}  // namespace emlabcpp
