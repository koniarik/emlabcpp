
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

        collector_test_fixture( std::string_view name, testing::collector& col )
          : testing::test_interface( name )
          , coll( col )
        {
        }

        testing::test_coroutine run( pmr::memory_resource&, testing::record& )
        {
                coll.set( 0, "key1", 42 );
                EMLABCPP_LOG( "Collected key1" );
                testing::node_id nid = co_await coll.set( 0, "key2", CONTIGUOUS_CONT_ARRAY );
                EMLABCPP_LOG( "Collected key2" );
                coll.append( nid, 52 );
                EMLABCPP_LOG( "Collected 52" );
                coll.append( nid, 666 );
                EMLABCPP_LOG( "Collected 666" );

                checkpoint_reached = true;

                co_await coll.set( nid, "key3", CONTIGUOUS_CONT_ARRAY );

                end_reached = true;
        }
};

TEST( collect, base )
{
        testing::collector*      col_ptr;
        testing::collect_server* server_ptr;

        auto col_send_f = [&]( auto, auto data ) {
                server_ptr->on_msg( data );
        };
        auto server_send_f = [&]( auto, auto data ) {
                col_ptr->on_msg( data );
        };

        testing::collector coll{ 0, col_send_f };
        col_ptr = &coll;

        testing::collect_server server{ 0, pmr::new_delete_resource(), server_send_f };
        server_ptr = &server;

        collector_test_fixture tf{ "wololo", coll };

        testing::executor exec{ 0u, pmr::new_delete_resource(), tf };

        while ( !exec.finished() ) {
                exec.tick();
        }

        EXPECT_TRUE( tf.checkpoint_reached );
        EXPECT_FALSE( tf.end_reached );
}

}  // namespace emlabcpp