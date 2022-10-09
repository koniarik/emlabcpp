
#include "emlabcpp/experimental/request_reply.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

using output_type = std::string;
using input_type  = int;

request_reply< input_type, output_type > one_yield( pool_interface* )
{
        int ret = co_yield std::string{ "test" };

        std::ignore = ret;
}

TEST( RequestReply, base )
{

        pool_resource< 512, 1 >                  pool;
        request_reply< input_type, output_type > req = one_yield( &pool );

        EXPECT_NE( req.get_output(), nullptr );
        EXPECT_EQ( *req.get_output(), "test" );

        EXPECT_FALSE( req.tick() );

        req.store_input( 42 );

        EXPECT_TRUE( req.tick() );

        EXPECT_FALSE( req.tick() );
}

}  // namespace emlabcpp
