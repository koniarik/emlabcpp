#include "emlabcpp/protocol/message.h"
#include "emlabcpp/view.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

TEST( Protocol, message_conversion )
{
        std::array< uint8_t, 4 > data = { 53, 3, 64, 85 };
        protocol_message< 4 >    m{ data };

        auto buff = std::array< uint8_t, 8 >( m );

        EXPECT_EQ( view_n( buff.begin(), 4 ), view{ data } );
}
