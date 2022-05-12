
#include "emlabcpp/iterators/numeric.h"

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
