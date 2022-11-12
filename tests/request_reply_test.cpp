
#include "emlabcpp/experimental/coro/request_reply.h"

#include "emlabcpp/experimental/coro/round_robin_executor.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

using reply_type   = int;
using request_type = std::string;

using rr_coro = coro::request_reply< request_type, reply_type >;

rr_coro one_yield( pool_interface*, std::string val )
{
        int ret = co_yield val;

        std::ignore = ret;
}

TEST( RequestReply, base )
{

        pool_resource< 512, 1 > pool;
        rr_coro                 req = one_yield( &pool, "test" );

        EXPECT_NE( req.get_request(), nullptr );
        EXPECT_EQ( *req.get_request(), "test" );

        EXPECT_FALSE( req.tick() );

        req.store_reply( 42 );

        EXPECT_TRUE( req.tick() );

        EXPECT_FALSE( req.tick() );
}

TEST( RequestReply, executor )
{

        pool_resource< 512, 16 > pool;

        rr_coro cor = round_robin_run(
            &pool, std::array{ one_yield( &pool, "test1" ), one_yield( &pool, "test2" ) } );

        EXPECT_EQ( *cor.get_request(), "test1" );
        cor.store_reply( 42 );

        EXPECT_TRUE( cor.tick() );

        EXPECT_EQ( *cor.get_request(), "test2" );
        cor.store_reply( 666 );

        EXPECT_TRUE( cor.tick() );
        EXPECT_FALSE( cor.tick() );
}

}  // namespace emlabcpp
