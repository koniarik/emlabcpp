/// MIT License
///
/// Copyright (c) 2025-2026 Jan Veverak Koniarik
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

#include "emlabcpp/physical_quantity.h"

#include <cstdint>
#include <gtest/gtest.h>
#include <string>
#include <string_view>
#include <unordered_set>

namespace emlabcpp
{

// NOLINTNEXTLINE
TEST( physical_quantity_test, basic )
{
        unitless u{ 0.5f };

        EXPECT_EQ( *u, 0.5f );
        EXPECT_EQ( u += unitless{ 1.f }, unitless{ 1.5f } );
        EXPECT_EQ( u -= unitless{ 1.f }, unitless{ 0.5f } );
        EXPECT_EQ( u /= 2.f, unitless{ 0.25f } );
        EXPECT_EQ( u *= 2.f, unitless{ 0.5f } );
        EXPECT_EQ( float{ u }, 0.5f );
        EXPECT_EQ( u + unitless{ 1.f }, unitless{ 1.5f } );
        EXPECT_EQ( u - unitless{ 1.f }, unitless{ -0.5f } );
        EXPECT_EQ( -u, unitless{ -0.5f } );
        EXPECT_TRUE( u == unitless{ 0.5f } );
        EXPECT_FALSE( u == unitless{ 0.25f } );
        EXPECT_TRUE( u < unitless{ 0.75f } );
        EXPECT_FALSE( u < unitless{ 0.5f } );
        EXPECT_EQ( u * 0.5f, unitless{ 0.25f } );
        EXPECT_EQ( u / 0.5f, unitless{ 1.f } );
        EXPECT_EQ( abs( unitless{ -0.5f } ), unitless{ 0.5f } );
        EXPECT_EQ( cos( unitless{ 0.f } ), 1. );
        EXPECT_EQ( sin( unitless{ 0.f } ), 0. );
        EXPECT_EQ( max( unitless{ 0.5f }, unitless{ 1.f } ), unitless{ 1.f } );
        EXPECT_EQ( min( unitless{ 0.5f }, unitless{ 1.f } ), unitless{ 0.5f } );
        EXPECT_EQ( pow< 2 >( unitless{ 2.f } ), unitless{ 4.f } );
}

// NOLINTNEXTLINE
TEST( physical_quantity_test, specializations )
{
        EXPECT_EQ(
            *std::numeric_limits< unitless >::lowest(), std::numeric_limits< float >::lowest() );
        EXPECT_EQ( *std::numeric_limits< unitless >::min(), std::numeric_limits< float >::min() );
        EXPECT_EQ( *std::numeric_limits< unitless >::max(), std::numeric_limits< float >::max() );
}

TEST( physical_quantity_test, units )
{
        EXPECT_EQ( unitless::get_unit(), "" );
        EXPECT_EQ( length::get_unit(), "m" );
        EXPECT_EQ( mass::get_unit(), "kg" );
        EXPECT_EQ( timeq::get_unit(), "s" );
        EXPECT_EQ( current::get_unit(), "A" );
        EXPECT_EQ( temp::get_unit(), "K" );
        EXPECT_EQ( amount_of_substance::get_unit(), "mol" );
        EXPECT_EQ( luminous_intensity::get_unit(), "cd" );
        EXPECT_EQ( angle::get_unit(), "rad" );
        EXPECT_EQ( byte::get_unit(), "B" );
        EXPECT_EQ( resistance::get_unit(), "m^2kgs^-3A^-2" );
}

TEST( physical_quantity_test, hash )
{
        EXPECT_EQ(
            std::hash< length >{}( length{ 2.F } ),
            std::hash< float >{}( 2.F ) ^ std::hash< std::string_view >{}( "m" ) );

        std::unordered_set< length > const lengths{ length{ 1.F }, length{ 1.F }, length{ 2.F } };
        EXPECT_EQ( lengths.size(), 2 );

        using apple_count = tagged_quantity< struct apple_count_tag, uint32_t >;
        EXPECT_EQ(
            std::hash< apple_count >{}( apple_count{ 3U } ),
            std::hash< uint32_t >{}( 3U ) ^ std::hash< std::string >{}( "" ) );
}

}  // namespace emlabcpp
