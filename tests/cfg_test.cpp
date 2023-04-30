
#include "emlabcpp/experimental/cfg/handler.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

struct kval
{
        int key;
        int val;
};

using handler = cfg::handler< int, kval >;

TEST( CFG, store )
{
        std::array< std::byte, 1024 > target_buffer;
        uint32_t                      id      = 1;
        int                           payload = 666;
        std::vector< kval >           fields;
        auto                          chcksm_f = [&]( std::span< std::byte > ) -> cfg::checksum {
                return 0;
        };

        bool succ = handler::store( target_buffer, id, payload, data_view( fields ), chcksm_f );
        EXPECT_TRUE( succ );
}

}  // namespace emlabcpp
