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

#include "emlabcpp/algorithm.h"
#include "emlabcpp/match.h"
#include "emlabcpp/protocol/message.h"
#include "emlabcpp/protocol/sequencer.h"
#include "emlabcpp/protocol/streams.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

struct sequencer_def
{
        using message_type = protocol::message< 16 >;

        static constexpr protocol::message< 2 > prefix{ 0x44, 0x44 };
        static constexpr std::size_t            fixed_size  = 3;
        static constexpr std::size_t            buffer_size = message_type::capacity * 2;

        static constexpr std::size_t get_size( auto const& bview )
        {
                auto iter = bview.begin();
                std::advance( iter, 2 );
                return std::to_integer< std::size_t >( *iter ) + fixed_size;
        }
};

using sequencer = protocol::sequencer< sequencer_def >;

TEST( protocol_seq, basic )
{
        protocol::message< 6 > data{ 0x44, 0x44, 0x03, 0x02, 0x03, 0xe8 };

        sequencer seq;

        seq.insert( view_n( data.begin(), 3 ) );
        match(
            seq.get_message(),
            [&]( protocol::sequencer_read_request to_read ) {
                    EXPECT_EQ( *to_read, 3 );
            },
            [&]( auto ) {
                    FAIL();
            } );

        seq.insert( view_n( data.begin() + 3, 3 ) );
        match(
            seq.get_message(),
            [&]( protocol::sequencer_read_request ) {
                    FAIL();
            },
            [&]( auto msg ) {
                    bool const are_equal = equal( msg, data );
                    EXPECT_TRUE( are_equal );
            } );
}

TEST( protocol_seq, noise_at_start )
{
        protocol::message< 7 > data{ 0x32, 0x44, 0x44, 0x03, 0x02, 0x03, 0xe8 };

        sequencer seq;

        seq.insert( view_n( data.begin(), 4 ) );
        match(
            seq.get_message(),
            [&]( protocol::sequencer_read_request to_read ) {
                    EXPECT_EQ( *to_read, 3 );
            },
            [&]( auto ) {
                    FAIL();
            } );

        seq.insert( view_n( data.begin() + 4, 3 ) );
        match(
            seq.get_message(),
            [&]( protocol::sequencer_read_request ) {
                    FAIL();
            },
            [&]( auto msg ) {
                    bool const are_equal = equal( msg, tail( data ) );
                    EXPECT_TRUE( are_equal ) << msg;
            } );
}

TEST( protocol_seq, multi_msg )
{
        protocol::message< 10 > data{ 0x32, 0x44, 0x44, 0x01, 0x42, 0x4, 0x44, 0x44, 0x01, 0x34 };

        auto msg1 = view_n( data.begin() + 1, 4 );
        auto msg2 = view_n( data.begin() + 6, 4 );

        sequencer seq;

        seq.insert( view_n( data.begin(), 5 ) );
        match(
            seq.get_message(),
            [&]( protocol::sequencer_read_request ) {
                    FAIL();
            },
            [&]( auto msg ) {
                    bool const are_equal = equal( msg, msg1 );
                    EXPECT_TRUE( are_equal );
            } );

        seq.insert( view_n( data.begin() + 4, 6 ) );
        match(
            seq.get_message(),
            [&]( protocol::sequencer_read_request ) {
                    FAIL();
            },
            [&]( auto msg ) {
                    bool const are_equal = equal( msg, msg2 );
                    EXPECT_TRUE( are_equal );
            } );
}

}  // namespace emlabcpp
