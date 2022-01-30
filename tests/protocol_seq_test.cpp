#include "emlabcpp/algorithm.h"
#include "emlabcpp/protocol/message.h"
#include "emlabcpp/protocol/sequencer.h"
#include "emlabcpp/protocol/streams.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

struct sequencer_def
{
        using message_type = protocol_message< 16 >;

        static constexpr std::array< uint8_t, 2 > prefix      = { 0x44, 0x44 };
        static constexpr std::size_t              fixed_size  = 3;
        static constexpr std::size_t              buffer_size = message_type::max_size * 2;

        static constexpr std::size_t get_size( const auto& bview )
        {
                return bview[2] + fixed_size;
        }
};

using sequencer = protocol_sequencer< sequencer_def >;

TEST( protocol_seq, basic )
{
        std::array< uint8_t, 6 > data = { 0x44, 0x44, 0x03, 0x02, 0x03, 0xe8 };

        sequencer seq;

        seq.load_data( view_n( data.begin(), 3 ) )
            .match(
                [&]( std::size_t to_read ) {
                        EXPECT_EQ( to_read, 3 );
                },
                [&]( auto ) {
                        FAIL();
                } );

        seq.load_data( view_n( data.begin() + 3, 3 ) )
            .match(
                [&]( std::size_t ) {
                        FAIL();
                },
                [&]( auto msg ) {
                        bool are_equal = equal( msg, data );
                        EXPECT_TRUE( are_equal );
                } );
}

TEST( protocol_seq, noise_at_start )
{
        std::array< uint8_t, 7 > data = { 0x32, 0x44, 0x44, 0x03, 0x02, 0x03, 0xe8 };

        sequencer seq;

        seq.load_data( view_n( data.begin(), 4 ) )
            .match(
                [&]( std::size_t to_read ) {
                        EXPECT_EQ( to_read, 3 );
                },
                [&]( auto ) {
                        FAIL();
                } );

        seq.load_data( view_n( data.begin() + 4, 3 ) )
            .match(
                [&]( std::size_t ) {
                        FAIL();
                },
                [&]( auto msg ) {
                        bool are_equal = equal( msg, tail( data ) );
                        EXPECT_TRUE( are_equal ) << msg;
                } );
}

TEST( protocol_seq, multi_msg )
{
        std::array< uint8_t, 10 > data = {
            0x32, 0x44, 0x44, 0x01, 0x42, 0x4, 0x44, 0x44, 0x01, 0x34 };

        auto msg1 = view_n( data.begin() + 1, 4 );
        auto msg2 = view_n( data.begin() + 6, 4 );

        sequencer seq;

        seq.load_data( view_n( data.begin(), 5 ) )
            .match(
                [&]( std::size_t ) {
                        FAIL();
                },
                [&]( auto msg ) {
                        bool are_equal = equal( msg, msg1 );
                        EXPECT_TRUE( are_equal );
                } );

        seq.load_data( view_n( data.begin() + 4, 6 ) )
            .match(
                [&]( std::size_t ) {
                        FAIL();
                },
                [&]( auto msg ) {
                        bool are_equal = equal( msg, msg2 );
                        EXPECT_TRUE( are_equal );
                } );
}
