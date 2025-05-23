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
