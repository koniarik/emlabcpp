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

#include "emlabcpp/static_circular_buffer.h"

#include "emlabcpp/algorithm.h"
#include "operations_counter.h"

#include <gtest/gtest.h>

using namespace emlabcpp;
using namespace std::literals;

static constexpr std::size_t buffer_size = 7;

using trivial_buffer   = static_circular_buffer< int, buffer_size >;
using trivial_iterator = typename trivial_buffer::iterator;
using obj_buffer       = static_circular_buffer< std::string, buffer_size >;

TEST( static_circular_buffer_test, pop_front )
{
        trivial_buffer tbuff;
        obj_buffer     obuff;

        tbuff.push_back( 33 );
        tbuff.push_back( 42 );

        EXPECT_EQ( tbuff.front(), 33 );
        tbuff.pop_front();
        EXPECT_EQ( tbuff.front(), 42 );

        obuff.push_back( "Nope." );
        obuff.push_back( "Yes" );

        EXPECT_EQ( obuff.front(), "Nope." );
        obuff.pop_front();
        EXPECT_EQ( obuff.front(), "Yes" );
}

TEST( static_circular_buffer_test, push_back )
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

TEST( static_circular_buffer_test, emplace_back )
{
        trivial_buffer tbuff;
        obj_buffer     obuff;

        tbuff.emplace_back( 42 );
        EXPECT_EQ( tbuff.back(), 42 );

        // This is special constructor of std::string
        const std::size_t i = 5;
        obuff.emplace_back( i, 'c' );
        EXPECT_EQ( obuff.back(), "ccccc" );
}

TEST( static_circular_buffer_test, circler_overflow_trivial )
{
        const std::vector< int > tidata = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        std::vector< int >       todata;
        trivial_buffer           tbuff;

        EXPECT_GT( tidata.size(), buffer_size );

        // insert data from tidata into tbuff, if it is full start poping the
        // data from tbuff to todata
        for ( const int i : tidata ) {
                if ( tbuff.full() ) {
                        todata.push_back( tbuff.front() );
                        tbuff.pop_front();
                }
                tbuff.push_back( i );
        }
        // pop rest of the data in tbuff into todata
        while ( !tbuff.empty() ) {
                todata.push_back( tbuff.front() );
                tbuff.pop_front();
        }

        // tbuff should be FIFO so the input/output dat should be equal
        EXPECT_EQ( tidata, todata );
}

TEST( static_circular_buffer_test, circle_overflow_object )
{
        const std::vector< std::string > oidata = {
            "Pack", "my", "box", "with", "five", "dozen", "liquor", "jugs." };
        std::vector< std::string > oodata;
        obj_buffer                 obuff;

        EXPECT_GT( oidata.size(), buffer_size );

        for ( const std::string& s : oidata ) {
                if ( obuff.full() ) {
                        oodata.push_back( obuff.front() );
                        obuff.pop_front();
                }
                obuff.push_back( s );
        }

        while ( !obuff.empty() ) {
                oodata.push_back( obuff.front() );
                obuff.pop_front();
        }

        EXPECT_EQ( oidata, oodata );
}

TEST( static_circular_buffer_test, usage )
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

TEST( static_circular_buffer_test, copy_trivial )
{
        trivial_buffer tbuff;
        for ( const int i : { 1, 2, 3, 4, 5 } ) {
                tbuff.push_back( i );
        }

        trivial_buffer cpy{ tbuff };

        EXPECT_EQ( tbuff, cpy );

        tbuff.pop_front();
        cpy.pop_front();

        EXPECT_EQ( tbuff, cpy );

        tbuff.push_back( 42 );

        EXPECT_NE( tbuff, cpy );

        trivial_buffer cpy2;
        cpy2 = tbuff;

        EXPECT_EQ( cpy2, tbuff );
}

TEST( static_circular_buffer_test, copy_object )
{
        obj_buffer obuff;
        for ( const std::string s :
              { "Pack", "my", "box", "with", "five", "dozen", "liquor", "jugs." } ) {
                obuff.push_back( s );
        }

        const obj_buffer cpy{ obuff };
        EXPECT_EQ( obuff, cpy );

        obj_buffer cpy2;
        cpy2 = obuff;
        EXPECT_EQ( obuff, cpy2 );
}

TEST( static_circular_buffer_test, move_trivial )
{
        trivial_buffer tbuff;
        for ( const int i : { 1, 2, 3, 4, 5 } ) {
                tbuff.push_back( i );
        }

        const trivial_buffer cpy{ tbuff };
        trivial_buffer       moved{ std::move( tbuff ) };

        EXPECT_EQ( cpy, moved );

        trivial_buffer moved2;
        moved2 = std::move( moved );

        EXPECT_EQ( cpy, moved2 );
}

TEST( static_circular_buffer_test, move_object )
{
        obj_buffer obuff;
        for ( const std::string s :
              { "Pack", "my", "box", "with", "five", "dozen", "liquor", "jugs." } ) {
                obuff.push_back( s );
        }

        const obj_buffer cpy{ obuff };
        obj_buffer       moved{ std::move( obuff ) };

        EXPECT_EQ( cpy, moved );

        obj_buffer moved2;
        moved2 = std::move( moved );

        EXPECT_EQ( cpy, moved2 );
}

TEST( static_circular_buffer_test, iterators )
{
        trivial_buffer           tbuff;
        const std::vector< int > data = { 1, 2, 3, 4, 5 };
        for ( const int i : data ) {
                tbuff.push_back( i );
        }

        std::vector< int > res;
        for ( const int i : tbuff ) {
                res.push_back( i );
        }

        const bool are_equal = equal( data, res );
        EXPECT_TRUE( are_equal );

        const trivial_iterator beg = tbuff.begin();
        const trivial_iterator cpy{ beg };
        EXPECT_EQ( beg, cpy );
}

TEST( static_circular_buffer_test, view )
{
        obj_buffer obuff;
        for ( const std::string& s : { "1"s, "2"s, "3"s, "4"s } ) {
                obuff.push_back( s );
        }
        std::stringstream ss;

        ss << view{ obuff };

        EXPECT_EQ( ss.str(), "1,2,3,4" );
}

TEST( static_circular_buffer_test, back_inserter )
{
        trivial_buffer       buff;
        std::array< int, 3 > data = { 1, 2, 3 };

        copy( data, std::back_inserter( buff ) );

        data = { 4, 5, 6 };

        copy( data, std::back_inserter( buff ) );

        std::array< int, 6 > expected = { 1, 2, 3, 4, 5, 6 };

        const bool are_equal = equal( buff, expected );
        EXPECT_TRUE( are_equal ) << "buff: " << view{ buff } << "\n"
                                 << "expected: " << view{ expected } << "\n";
}

TEST( static_circular_buffer_test, back )
{
        trivial_buffer tbuff;
        for ( const int i : { 1, 2, 3, 4, 5, 6, 7 } ) {
                tbuff.push_back( i );
        }

        EXPECT_TRUE( tbuff.full() );

        tbuff.pop_front();
        tbuff.push_back( 8 );

        EXPECT_TRUE( tbuff.full() );
        EXPECT_EQ( tbuff.back(), 8 );
}

struct operations_counter_circular_buffer
{
        using container_type           = static_circular_buffer< operations_counter, 32 >;
        static constexpr std::size_t n = 16;
};

using types = ::testing::Types< operations_counter_circular_buffer >;
INSTANTIATE_TYPED_TEST_SUITE_P(
    operations_counter_circular_buffer_fixture,
    operations_counter_fixture,
    types );
