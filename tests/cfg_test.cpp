#include "emlabcpp/convert_view.h"
#include "emlabcpp/experimental/cfg/handler.h"
#include "emlabcpp/experimental/logging.h"
#include "emlabcpp/protocol/message.h"
#include "emlabcpp/protocol/streams.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

struct kval
{
        uint16_t key;
        uint16_t val;
};

using handler = cfg::handler< uint32_t, kval, std::endian::big >;

cfg::checksum chcksm_f( std::span< std::byte > data )
{
        cfg::checksum def = 0x0A0A0A0A;
        return accumulate( data, def, []( cfg::checksum val, std::byte b ) -> cfg::checksum {
                return std::rotl( val, sizeof( b ) ) ^ static_cast< cfg::checksum >( b );
        } );
}

TEST( CFG, combined )
{
        std::array< std::byte, 1024 > target_buffer{};
        uint32_t                      payload = 666;
        std::vector< kval >           fields{ kval{ 1, 42 }, kval{ 2, 666 } };

        for ( std::size_t n : { 1u, 8u, 16u } ) {
                bool succ = handler::store(
                    view_n( target_buffer.data(), n ), payload, data_view( fields ), chcksm_f );
                EXPECT_FALSE( succ );
        }
        bool succ = handler::store( target_buffer, payload, data_view( fields ), chcksm_f );
        EXPECT_TRUE( succ );

        protocol::message< 128 > expected{
            0xA0, 0xA0, 0xA0, 0xA2,  //
            0x00, 0x00, 0x00, 0x02,  // header
            0xA0, 0xA0, 0xA0, 0x3E,  //
            0x00, 0x00, 0x02, 0x9A,  // payload
            0xA0, 0xA0, 0xA0, 0x8E,  //
            0x00, 0x01, 0x00, 0x2A,  // f1
            0xA0, 0xA0, 0xA0, 0x36,  //
            0x00, 0x02, 0x02, 0x9A,  // f2
            0x00, 0x00, 0x00, 0x00,  //
        };
        protocol::message< 128 > sub{ view_n( target_buffer.begin(), expected.size() ) };
        EXPECT_EQ( expected, sub ) << expected << "\n" << sub << "\n";

        cfg::load_result lr = handler::load(
            target_buffer,
            [&]( const uint32_t& pl ) -> bool {
                    EXPECT_EQ( pl, payload );
                    return true;
            },
            [&]( const kval& ) {},
            chcksm_f );
        EXPECT_EQ( lr, cfg::load_result::SUCCESS ) << convert_enum( lr );
}

}  // namespace emlabcpp
