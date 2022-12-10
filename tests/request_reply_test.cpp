#include "emlabcpp/experimental/coro/request_reply.h"

#include "emlabcpp/experimental/coro/round_robin_executor.h"
#include "emlabcpp/pmr/pool_resource.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

using reply_type   = int;
using request_type = std::string;

using rr_coro = coro::request_reply< request_type, reply_type >;

rr_coro one_yield( pmr::memory_resource&, std::string val )
{
        int ret = co_yield val;

        std::ignore = ret;
}

TEST( RequestReply, base )
{

        pmr::pool_resource< 512, 1 > mem_resource;
        rr_coro                      req = one_yield( mem_resource, "test" );

        EXPECT_NE( req.get_request(), nullptr );
        EXPECT_EQ( *req.get_request(), "test" );

        EXPECT_FALSE( req.tick() );

        req.store_reply( 42 );

        EXPECT_TRUE( req.tick() );

        EXPECT_FALSE( req.tick() );
}

TEST( RequestReply, executor )
{

        pmr::pool_resource< 512, 16 > mem_resource;

        rr_coro cor = round_robin_run(
            mem_resource,
            std::array{ one_yield( mem_resource, "test1" ), one_yield( mem_resource, "test2" ) } );

        EXPECT_EQ( *cor.get_request(), "test1" );
        cor.store_reply( 42 );

        EXPECT_TRUE( cor.tick() );

        EXPECT_EQ( *cor.get_request(), "test2" );
        cor.store_reply( 666 );

        EXPECT_TRUE( cor.tick() );
        EXPECT_FALSE( cor.tick() );
}

}  // namespace emlabcpp
