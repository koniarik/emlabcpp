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

#include "emlabcpp/static_vector.h"

#include "./util/operations_counter.h"
#include "emlabcpp/algorithm.h"

#include <gtest/gtest.h>

namespace emlabcpp
{
using namespace std::literals;

static constexpr std::size_t buffer_size = 10;

using trivial_buffer   = static_vector< int, buffer_size >;
using trivial_iterator = typename trivial_buffer::iterator;
using obj_buffer       = static_vector< std::string, buffer_size >;

static_assert( std::regular< trivial_buffer > );
static_assert( std::swappable< trivial_buffer > );
static_assert( std::regular< obj_buffer > );
static_assert( std::swappable< obj_buffer > );

TEST( static_vector_test, pop_back )
{
        trivial_buffer tbuff;
        obj_buffer     obuff;

        tbuff.push_back( 33 );
        tbuff.push_back( 42 );

        EXPECT_EQ( tbuff.back(), 42 );
        tbuff.pop_back();
        EXPECT_EQ( tbuff.back(), 33 );

        obuff.push_back( "Nope." );
        obuff.push_back( "Yes" );

        EXPECT_EQ( obuff.back(), "Yes" );
        obuff.pop_back();
        EXPECT_EQ( obuff.back(), "Nope." );
}

TEST( static_vector_test, push_back )
{
        trivial_buffer tbuff;
        obj_buffer     obuff;

        // insert into empty buffers

        tbuff.push_back( 42 );
        EXPECT_EQ( tbuff.back(), 42 );

        obuff.push_back( "The five boxing wizards jump quickly." );
        EXPECT_EQ( obuff.back(), "The five boxing wizards jump quickly." );

        // inser into non-empty buffers

        tbuff.push_back( -1 );
        EXPECT_EQ( tbuff.back(), -1 );

        obuff.push_back( "Nope." );
        EXPECT_EQ( obuff.back(), "Nope." );
}

TEST( static_vector_test, emplace_back )
{
        trivial_buffer tbuff;
        obj_buffer     obuff;

        tbuff.emplace_back( 42 );
        EXPECT_EQ( tbuff.back(), 42 );

        // This is special constructor of std::string
        std::size_t const i = 5;
        obuff.emplace_back( i, 'c' );
        EXPECT_EQ( obuff.back(), "ccccc" );
}

TEST( static_vector_test, usage )
{
        trivial_buffer tbuff;

        EXPECT_TRUE( tbuff.empty() );
        EXPECT_FALSE( tbuff.full() );
        EXPECT_EQ( tbuff.size(), 0 );
        EXPECT_EQ( tbuff.max_size(), buffer_size );

        tbuff.push_back( 42 );

        EXPECT_FALSE( tbuff.empty() );
        EXPECT_FALSE( tbuff.full() );
        EXPECT_EQ( tbuff.size(), 1 );
}

TEST( static_vector_test, copy_trivial )
{
        trivial_buffer tbuff;
        for ( int const i : { 1, 2, 3, 4, 5 } )
                tbuff.push_back( i );

        trivial_buffer cpy{ tbuff };

        EXPECT_EQ( tbuff, cpy );

        tbuff.pop_back();
        cpy.pop_back();

        EXPECT_EQ( tbuff, cpy );

        tbuff.push_back( 42 );

        EXPECT_NE( tbuff, cpy );

        trivial_buffer cpy2;
        cpy2 = tbuff;

        EXPECT_EQ( cpy2, tbuff );
}

TEST( static_vector_test, copy_object )
{
        obj_buffer obuff;
        for ( std::string const s :
              { "Pack", "my", "box", "with", "five", "dozen", "liquor", "jugs." } ) {
                obuff.push_back( s );
        }

        obj_buffer const cpy{ obuff };
        EXPECT_EQ( obuff, cpy );

        obj_buffer cpy2;
        cpy2 = obuff;
        EXPECT_EQ( obuff, cpy2 );
}

TEST( static_vector_test, move_trivial )
{
        trivial_buffer tbuff;
        for ( int const i : { 1, 2, 3, 4, 5 } )
                tbuff.push_back( i );

        trivial_buffer const cpy{ tbuff };
        trivial_buffer       moved{ std::move( tbuff ) };

        EXPECT_EQ( cpy, moved );

        trivial_buffer moved2;
        moved2 = std::move( moved );

        EXPECT_EQ( cpy, moved2 );
}

TEST( static_vector_test, move_object )
{
        obj_buffer obuff;
        for ( std::string const s :
              { "Pack", "my", "box", "with", "five", "dozen", "liquor", "jugs." } ) {
                obuff.push_back( s );
        }

        obj_buffer const cpy{ obuff };
        obj_buffer       moved{ std::move( obuff ) };

        EXPECT_EQ( cpy, moved );

        obj_buffer moved2;
        moved2 = std::move( moved );

        EXPECT_EQ( cpy, moved2 );
}

TEST( static_vector_test, iterators )
{
        trivial_buffer           tbuff;
        std::vector< int > const data = { 1, 2, 3, 4, 5 };
        for ( int const i : data )
                tbuff.push_back( i );

        std::vector< int > res;
        for ( int const i : tbuff )
                res.push_back( i );

        bool const are_equal = equal( data, res );
        EXPECT_TRUE( are_equal );

        trivial_iterator beg = tbuff.begin();
        trivial_iterator cpy{ beg };
        EXPECT_EQ( beg, cpy );
}

TEST( static_vector_test, swap )
{
        obj_buffer vec1{ std::array{ "1"s, "2"s, "3"s, "4"s, "5"s } };
        obj_buffer vec2{ std::array{ "a"s, "b"s, "c"s } };

        obj_buffer const vec1c = vec1;
        obj_buffer const vec2c = vec2;

        EXPECT_EQ( vec1.size(), 5 );
        EXPECT_EQ( vec2.size(), 3 );
        vec1.swap( vec2 );
        EXPECT_EQ( vec1.size(), 3 );
        EXPECT_EQ( vec2.size(), 5 );

        EXPECT_EQ( vec1c, vec2 );
        EXPECT_EQ( vec2c, vec1 );

        swap( vec1, vec2 );

        EXPECT_EQ( vec1c, vec1 );
        EXPECT_EQ( vec2c, vec2 );
}

TEST( static_vector_test, view )
{
        obj_buffer        vec1{ std::array{ "1"s, "2"s, "3"s, "4"s } };
        std::stringstream ss;

        ss << view{ vec1 };

        EXPECT_EQ( ss.str(), "1,2,3,4" );
}

struct operations_counter_static_vector
{
        using container_type           = static_vector< operations_counter, 32 >;
        static constexpr std::size_t n = 16;
};

using types = ::testing::Types< operations_counter_static_vector >;
INSTANTIATE_TYPED_TEST_SUITE_P(
    operations_counter_static_vector_fixture,
    operations_counter_fixture,
    types );

}  // namespace emlabcpp
