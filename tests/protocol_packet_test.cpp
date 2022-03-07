
#include "emlabcpp/protocol/packet.h"
#include "emlabcpp/protocol/packet_handler.h"
#include "emlabcpp/protocol/streams.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

struct protocol_packet_test_def
{
        static constexpr protocol_endianess_enum  endianess = PROTOCOL_BIG_ENDIAN;
        static constexpr std::array< uint8_t, 4 > prefix    = { 0x91, 0x19, 0x91, 0x19 };
        using size_type                                     = uint16_t;
        using checksum_type                                 = uint16_t;

        static constexpr checksum_type get_checksum( const view< const uint8_t* > )
        {
                return 0x00;
        }
};

using payload      = protocol_tuple< PROTOCOL_BIG_ENDIAN, uint32_t, uint8_t, uint8_t >;
using packet       = protocol_packet< protocol_packet_test_def, payload >;
using message_type = typename packet::message_type;
using handler      = protocol_packet_handler< packet >;

TEST( Packet, simple )
{
        std::tuple< uint32_t, uint8_t, uint8_t > val{ 0x43434343, 0x8, 0x16 };
        message_type                             msg = handler::serialize( val );
        message_type                             res{
            0x91, 0x19, 0x91, 0x19, 0x00, 0x08, 0x43, 0x43, 0x43, 0x43, 0x08, 0x16, 0x00, 0x00 };

        EXPECT_EQ( msg, res );

        handler::extract( msg ).match(
            [&]( std::tuple< uint32_t, uint8_t, uint8_t > pack ) {
                    EXPECT_EQ( pack, val );
            },
            [&]( protocol_error_record e ) {
                    FAIL() << e;
            } );
}

TEST( Packet, seq )
{
        message_type msg{
            0x91, 0x19, 0x91, 0x19, 0x00, 0x08, 0x43, 0x43, 0x43, 0x43, 0x08, 0x16, 0x00, 0x00 };

        using seq = typename packet::sequencer;

        seq test_seq{};

        test_seq.load_data( view{ msg } )
            .match(
                [&]( std::size_t ) {
                        FAIL();
                },
                [&]( message_type newmsg ) {
                        EXPECT_EQ( newmsg, msg );
                } );
}
