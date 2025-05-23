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

#include "emlabcpp/match.h"
#include "emlabcpp/protocol/packet.h"
#include "emlabcpp/protocol/packet_handler.h"
#include "emlabcpp/protocol/streams.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

struct packet_test_def
{
        static constexpr std::endian                endianess = std::endian::big;
        static constexpr std::array< std::byte, 4 > prefix    = bytes( 0x91, 0x19, 0x91, 0x19 );
        using size_type                                       = uint16_t;
        using checksum_type                                   = uint16_t;

        static constexpr checksum_type get_checksum( view< std::byte const* > const )
        {
                return 0x00;
        }
};

using payload      = protocol::tuple< std::endian::big, uint32_t, uint8_t, uint8_t >;
using packet       = protocol::packet< packet_test_def, payload >;
using message_type = typename packet::message_type;
using handler      = protocol::packet_handler< packet >;

TEST( Packet, simple )
{
        std::tuple< uint32_t, uint8_t, uint8_t > val{ 0x43434343, 0x8, 0x16 };
        message_type                             msg = handler::serialize( val );
        message_type const                       res(
            0x91, 0x19, 0x91, 0x19, 0x00, 0x06, 0x43, 0x43, 0x43, 0x43, 0x08, 0x16, 0x00, 0x00 );

        EXPECT_EQ( msg, res ) << "msg: " << msg << "\n"
                              << "res: " << res;

        match(
            handler::extract( msg ),
            [&]( std::tuple< uint32_t, uint8_t, uint8_t > pack ) {
                    EXPECT_EQ( pack, val );
            },
            [&]( protocol::error_record e ) {
                    FAIL() << e;
            } );
}

TEST( Packet, seq )
{
        message_type msg{
            0x91, 0x19, 0x91, 0x19, 0x00, 0x06, 0x43, 0x43, 0x43, 0x43, 0x08, 0x16, 0x00, 0x00 };

        using seq = typename packet::sequencer_type;

        seq test_seq{};

        test_seq.insert( msg );
        match(
            test_seq.get_message(),
            [&]( protocol::sequencer_read_request ) {
                    FAIL();
            },
            [&]( message_type newmsg ) {
                    EXPECT_EQ( newmsg, msg );
            } );
}

}  // namespace emlabcpp
