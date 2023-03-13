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

#include "emlabcpp/range.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

struct operations_counter
{
        static std::size_t copy_count;     // NOLINT
        static std::size_t move_count;     // NOLINT
        static std::size_t destroy_count;  // NOLINT
        static std::size_t default_count;  // NOLINT

        operations_counter()
        {
                default_count += 1;
        }

        operations_counter( const operations_counter& )
        {
                copy_count += 1;
        }

        operations_counter( operations_counter&& ) noexcept
        {
                move_count += 1;
        }

        operations_counter& operator=( const operations_counter& )
        {
                copy_count += 1;
                return *this;
        }
        operations_counter& operator=( operations_counter&& ) noexcept
        {
                move_count += 1;
                return *this;
        }

        ~operations_counter()
        {
                destroy_count += 1;
        }

        static void reset()
        {
                operations_counter::move_count    = 0;
                operations_counter::copy_count    = 0;
                operations_counter::destroy_count = 0;
                operations_counter::default_count = 0;
        }
};
std::size_t operations_counter::move_count    = 0;  // NOLINT
std::size_t operations_counter::copy_count    = 0;  // NOLINT
std::size_t operations_counter::destroy_count = 0;  // NOLINT
std::size_t operations_counter::default_count = 0;  // NOLINT

template < typename T >
struct operations_counter_fixture : ::testing::Test
{
        using container = typename T::container_type;
        container                    cont;
        static constexpr std::size_t n = T::n;

        void SetUp() override
        {
                for ( std::size_t i : range( n ) ) {
                        std::ignore = i;
                        cont.emplace_back();
                }
                operations_counter::reset();
        }
};

TYPED_TEST_SUITE_P( operations_counter_fixture );

TYPED_TEST_P( operations_counter_fixture, copy_test )
{
        {
                auto cpy{ this->cont };
                EXPECT_EQ( cpy.size(), this->n );
        }

        EXPECT_EQ( this->cont.size(), this->n );
        EXPECT_EQ( operations_counter::move_count, 0 );
        EXPECT_EQ( operations_counter::copy_count, this->n );
        EXPECT_EQ( operations_counter::destroy_count, this->n );
        EXPECT_EQ( operations_counter::default_count, 0 );
}

TYPED_TEST_P( operations_counter_fixture, move_test )
{
        {
                auto moved{ std::move( this->cont ) };

                EXPECT_EQ( moved.size(), this->n );
        }
        EXPECT_EQ( this->cont.size(), 0 );
        EXPECT_EQ( operations_counter::move_count, this->n );
        EXPECT_EQ( operations_counter::copy_count, 0 );
        EXPECT_EQ( operations_counter::destroy_count, 2 * this->n );
        EXPECT_EQ( operations_counter::default_count, 0 );
}

TYPED_TEST_P( operations_counter_fixture, copy_assignment_test )
{
        {
                typename TypeParam::container_type cpy{};
                cpy = this->cont;

                EXPECT_EQ( cpy.size(), this->n );
        }

        EXPECT_EQ( this->cont.size(), this->n );
        EXPECT_EQ( operations_counter::move_count, 0 );
        EXPECT_EQ( operations_counter::copy_count, this->n );
        EXPECT_EQ( operations_counter::destroy_count, this->n );
        EXPECT_EQ( operations_counter::default_count, 0 );
}

TYPED_TEST_P( operations_counter_fixture, move_assignment_test )
{
        {
                typename TypeParam::container_type moved{};
                moved = std::move( this->cont );

                EXPECT_EQ( moved.size(), this->n );
        }
        EXPECT_EQ( this->cont.size(), 0 );
        EXPECT_EQ( operations_counter::move_count, this->n );
        EXPECT_EQ( operations_counter::copy_count, 0 );
        EXPECT_EQ( operations_counter::destroy_count, 2 * this->n );
        EXPECT_EQ( operations_counter::default_count, 0 );
}

REGISTER_TYPED_TEST_SUITE_P(
    operations_counter_fixture,
    copy_test,
    move_test,
    copy_assignment_test,
    move_assignment_test );

}  // namespace emlabcpp
