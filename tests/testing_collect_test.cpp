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

#include "emlabcpp/experimental/testing/collect.h"
#include "emlabcpp/experimental/testing/executor.h"
#include "emlabcpp/pmr/new_delete_resource.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

struct collector_test_fixture : public testing::test_interface
{
        testing::collector& coll;
        bool                checkpoint_reached = false;
        bool                end_reached        = false;

        collector_test_fixture( testing::collector& col )
          : coll( col )
        {
        }

        [[nodiscard]] std::string_view get_name() const override
        {
                return "wololo";
        }

        testing::test_coroutine run( pmr::memory_resource&, testing::record& ) override
        {
                coll.set( 0, "key1", 42 );
                EMLABCPP_INFO_LOG( "Collected key1" );
                testing::node_id nid =
                    co_await coll.set( 0, "key2", contiguous_container_type::ARRAY );
                EMLABCPP_INFO_LOG( "Collected key2" );
                coll.append( nid, 52 );
                EMLABCPP_INFO_LOG( "Collected 52" );
                coll.append( nid, 666 );
                EMLABCPP_INFO_LOG( "Collected 666" );

                checkpoint_reached = true;

                co_await coll.set( nid, "key3", contiguous_container_type::ARRAY );

                end_reached = true;
        }
};

TEST( collect, base )
{

        testing::collector*      col_ptr;
        testing::collect_server* server_ptr;

        auto col_send_f = [&]( auto, auto data ) {
                return server_ptr->on_msg( data );
        };
        auto server_send_f = [&]( auto, auto data ) {
                return col_ptr->on_msg( data );
        };

        testing::collector coll{ 0, col_send_f };
        col_ptr = &coll;

        testing::collect_server server{ 0, pmr::new_delete_resource(), server_send_f };
        server_ptr = &server;

        collector_test_fixture tf{ coll };

        testing::executor exec{ 0u, pmr::new_delete_resource(), tf };

        while ( !exec.finished() ) {
                exec.tick();
        }

        EXPECT_TRUE( tf.checkpoint_reached );
        EXPECT_FALSE( tf.end_reached );
}

}  // namespace emlabcpp
