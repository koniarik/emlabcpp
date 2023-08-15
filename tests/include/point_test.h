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

#include "emlabcpp/experimental/geom/point.h"

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

TYPED_TEST_SUITE( PointTest, test_types );

}  // namespace emlabcpp
