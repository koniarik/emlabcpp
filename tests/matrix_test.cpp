
#include "emlabcpp/experimental/matrix.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

TEST( Matrix, mult )
{
        matrix< 2, 3 > A;
        A[0] = { 2.f, 3.f, 4.f };
        A[1] = { 1.f, 0.f, 0.f };

        matrix< 3, 2 > B;
        B[0] = { 0.f, 1000.f };
        B[1] = { 1.f, 100.f };
        B[2] = { 0.f, 10.f };

        matrix< 2, 2 > C = A * B;

        matrix< 2, 2 > expected;
        expected[0] = { 3, 2340 };
        expected[1] = { 0, 1000 };

        EXPECT_EQ( C, expected );
}

TEST( Matrix, sum )
{
        matrix< 2, 3 > A;
        A[0] = { 2.f, 3.f, 4.f };
        A[1] = { 1.f, 0.f, 0.f };

        matrix< 2, 3 > expected;
        expected[0] = { 4.f, 6.f, 8.f };
        expected[1] = { 2.f, 0.f, 0.f };

        matrix< 2, 3 > B = A + A;

        EXPECT_EQ( B, expected );
}

TEST( Matrix, transpose )
{
        matrix< 2, 3 > A;
        A[0] = { 2.f, 3.f, 4.f };
        A[1] = { 1.f, 0.f, 0.f };

        matrix< 3, 2 > expected;
        expected[0] = { 2.f, 1.f };
        expected[1] = { 3.f, 0.f };
        expected[2] = { 4.f, 0.f };

        EXPECT_EQ( transpose( A ), expected );

        EXPECT_EQ( transpose( transpose( A ) ), A );
}

}  // namespace emlabcpp
