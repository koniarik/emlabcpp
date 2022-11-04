
#include "emlabcpp/experimental/coroutine/request_reply.h"

#include "emlabcpp/experimental/coroutine/round_robin_executor.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

using reply_type   = std::string;
using request_type = int;

using rr_coro = request_reply< request_type, reply_type >;

rr_coro one_yield( pool_interface*, std::string val )
{
        int ret = co_yield val;

        std::ignore = ret;
}

TEST( RequestReply, base )
{

        pool_resource< 512, 1 >                   pool;
        request_reply< request_type, reply_type > req = one_yield( &pool, "test" );

        EXPECT_NE( req.get_reply(), nullptr );
        EXPECT_EQ( *req.get_reply(), "test" );

        EXPECT_FALSE( req.tick() );

        req.store_request( 42 );

        EXPECT_TRUE( req.tick() );

        EXPECT_FALSE( req.tick() );
}

TEST( RequestReply, executor )
{

        pool_resource< 512, 16 > pool;

        rr_coro cor = round_robin_run(
            &pool, std::array{ one_yield( &pool, "test1" ), one_yield( &pool, "test2" ) } );

        EXPECT_EQ( *cor.get_reply(), "test1" );
        cor.store_request( 42 );

        EXPECT_TRUE( cor.tick() );

        EXPECT_EQ( *cor.get_reply(), "test2" );
        cor.store_request( 666 );

        EXPECT_TRUE( cor.tick() );
        EXPECT_FALSE( cor.tick() );
}

}  // namespace emlabcpp
