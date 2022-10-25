
#include "emlabcpp/experimental/coroutine/request_reply.h"

#include "emlabcpp/experimental/coroutine/round_robin_executor.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

using output_type = std::string;
using input_type  = int;

using rr_coro = request_reply< input_type, output_type >;

rr_coro one_yield( pool_interface*, std::string val )
{
        int ret = co_yield val;

        std::ignore = ret;
}

TEST( RequestReply, base )
{

        pool_resource< 512, 1 >                  pool;
        request_reply< input_type, output_type > req = one_yield( &pool, "test" );

        EXPECT_NE( req.get_output(), nullptr );
        EXPECT_EQ( *req.get_output(), "test" );

        EXPECT_FALSE( req.tick() );

        req.store_input( 42 );

        EXPECT_TRUE( req.tick() );

        EXPECT_FALSE( req.tick() );
}

TEST( RequestReply, executor )
{
        using exec_type = round_robin_executor< rr_coro, 16 >;

        exec_type                exec;
        pool_resource< 512, 16 > pool;

        EXPECT_TRUE( exec.register_coroutine( one_yield( &pool, "test1" ) ) );
        EXPECT_TRUE( exec.register_coroutine( one_yield( &pool, "test2" ) ) );

        rr_coro cor = exec.run( &pool );

        EXPECT_EQ( *cor.get_output(), "test1" );
        cor.store_input( 42 );

        EXPECT_TRUE( cor.tick() );

        EXPECT_EQ( *cor.get_output(), "test2" );
        cor.store_input( 666 );

        EXPECT_TRUE( cor.tick() );
        EXPECT_FALSE( cor.tick() );
}

}  // namespace emlabcpp
