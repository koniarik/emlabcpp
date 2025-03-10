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
#include "emlabcpp/min_max.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

// NOLINTNEXTLINE
TEST( MinMax, min_max )
{
        const min_max< int > mm{ 0, 1 };
        EXPECT_EQ( mm.min(), 0 );
        EXPECT_EQ( mm.max(), 1 );

        auto [min_val, max_val] = mm;
        EXPECT_EQ( min_val, 0 );
        EXPECT_EQ( max_val, 1 );
}

// NOLINTNEXTLINE
TEST( MinMax, clamp )
{
        const min_max< float > mm{ -1.f, 1.f };

        EXPECT_EQ( clamp( 0.5f, mm ), 0.5f );
        EXPECT_EQ( clamp( -2.0f, mm ), -1.f );
        EXPECT_EQ( clamp( 2.0f, mm ), 1.f );
}

}  // namespace emlabcpp
