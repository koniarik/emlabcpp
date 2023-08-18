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
