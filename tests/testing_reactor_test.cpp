// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//
//  Copyright © 2022 Jan Veverak Koniarik
//  This file is part of project: emlabcpp
//
#include "test.h"

#include "emlabcpp/experimental/logging.h"
#include "emlabcpp/experimental/testing/controller.h"
#include "emlabcpp/experimental/testing/gtest.h"
#include "emlabcpp/experimental/testing/json.h"
#include "emlabcpp/experimental/testing/reactor.h"
#include "emlabcpp/protocol/streams.h"
#include "util.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

class reactor_interface : public testing::reactor_interface
{
public:
        std::vector< testing::reactor_controller_variant > msgs;

        void transmit( std::span< uint8_t > msg )
        {
                using h = protocol::handler< testing::reactor_controller_group >;
                h::extract( view_n( msg.data(), msg.size() ) )
                    .match(
                        [&]( const auto& var ) {
                                msgs.push_back( var );
                        },
                        [&]( const auto& ) {
                                FAIL();
                        } );
        }
};

template < typename T >
void executor_test_run( T& tf )
{
        for ( std::size_t i : range( 1u, 8u ) ) {
                testing::executor exec{
                    static_cast< testing::test_id >( i ), pmr::new_delete_resource(), tf };

                while ( !exec.finished() ) {
                        exec.tick();
                }

                EXPECT_EQ( tf.setup_count, i );
                EXPECT_EQ( tf.run_count, i );
                EXPECT_EQ( tf.teardown_count, i );
        }
}

TEST( executor, simple_full_run )
{
        simple_test_fixture tf{ "test" };

        executor_test_run( tf );
}

TEST( executor, complex_full_run )
{
        complex_test_fixture tf{ "test" };

        executor_test_run( tf );
}

TEST( reactor, reactor_simple )
{
        reactor_interface iface;
        testing::reactor  rec{ "reac", iface };

        simple_test_fixture tf{ "simple" };
        simple_test_fixture tf2{ "test" };

        rec.register_test( tf );
        rec.register_test( tf2 );

        rec.on_msg( testing::exec_request{ .rid = 0, .tid = 0 } );

        for ( std::size_t i : range( 20u ) ) {
                std::ignore = i;
                rec.tick();
        }

        EXPECT_EQ( tf.setup_count, 0 );
        EXPECT_EQ( tf.run_count, 0 );
        EXPECT_EQ( tf.teardown_count, 0 );
        EXPECT_EQ( tf2.setup_count, 1 );
        EXPECT_EQ( tf2.run_count, 1 );
        EXPECT_EQ( tf2.teardown_count, 1 );

        EXPECT_EQ( iface.msgs.size(), 1 );
        EXPECT_TRUE( std::holds_alternative< testing::test_finished >( iface.msgs.back() ) );
}

}  // namespace emlabcpp
