///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
/// and associated documentation files (the "Software"), to deal in the Software without
/// restriction, including without limitation the rights to use, copy, modify, merge, publish,
/// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or
/// substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
/// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
/// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

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

class nested_test_fixture : public testing::test_interface
{
public:
        using testing::test_interface::test_interface;

        [[nodiscard]] std::string_view get_name() const override
        {
                return "nested test case";
        }

        testing::test_coroutine run( pmr::memory_resource& mem, testing::record& rec ) override
        {
                for ( std::size_t i : range( 1u, 8u ) ) {
                        co_await sub.run( mem, rec );

                        EXPECT_EQ( sub.run_count, i );
                }
        }

        complex_test_fixture sub{};
};

class reactor_interface
{
public:
        static std::vector< testing::reactor_controller_variant > msgs;

        void operator()( auto, std::span< const uint8_t > msg )
        {
                using h = protocol::handler< testing::reactor_controller_group >;
                h::extract( view_n( msg.data(), msg.size() ) )
                    .match(
                        [&]( const auto& var ) {
                                EMLABCPP_INFO_LOG( "Got a msg: ", var );
                                msgs.push_back( var );
                        },
                        [&]( const auto& err ) {
                                EMLABCPP_ERROR_LOG( "Got an error: ", err );
                                FAIL();
                        } );
        }
};
std::vector< testing::reactor_controller_variant > reactor_interface::msgs{};

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
        simple_test_fixture tf{};

        executor_test_run( tf );
}

TEST( executor, complex_full_run )
{
        complex_test_fixture tf{};

        executor_test_run( tf );
}

TEST( executor, nested )
{
        nested_test_fixture tf{};

        testing::executor exec{ 0, pmr::new_delete_resource(), tf };

        while ( !exec.finished() ) {
                exec.tick();
        }
}

TEST( reactor, reactor_simple )
{
        reactor_interface iface;
        testing::reactor  rec{ 0, "reac", iface };

        testing::test_unit< simple_test_fixture > tf{ rec };
        testing::test_unit< simple_test_fixture > tf2{ rec };

        rec.on_msg( testing::exec_request{ .rid = 0, .tid = 0 } );

        for ( std::size_t i : range( 20u ) ) {
                std::ignore = i;
                rec.tick();
        }

        EXPECT_EQ( tf->setup_count, 0 );
        EXPECT_EQ( tf->run_count, 0 );
        EXPECT_EQ( tf->teardown_count, 0 );
        EXPECT_EQ( tf2->setup_count, 1 );
        EXPECT_EQ( tf2->run_count, 1 );
        EXPECT_EQ( tf2->teardown_count, 1 );

        EXPECT_EQ( iface.msgs.size(), 1 );
        EXPECT_TRUE( std::holds_alternative< testing::test_finished >( iface.msgs.back() ) );
}

}  // namespace emlabcpp
