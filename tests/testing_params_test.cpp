#include "emlabcpp/experimental/testing/executor.h"
#include "emlabcpp/experimental/testing/json.h"
#include "emlabcpp/experimental/testing/parameters.h"
#include "emlabcpp/pmr/new_delete_resource.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

struct params_test_fixture : public testing::test_interface
{
        testing::parameters& params;
        bool                 checkpoint_reached = false;
        bool                 end_reached        = false;

        params_test_fixture( testing::parameters& param )
          : params( param )
        {
        }

        std::string_view get_name() const
        {
                return "wololo";
        }

        testing::test_coroutine run( pmr::memory_resource&, testing::record& )
        {
                testing::node_type_enum t =
                    co_await params.get_type( co_await params.get_child( 0, "pi" ) );
                EXPECT_EQ( t, CONTIGUOUS_TREE_VALUE );

                bool bval = co_await params.get_value< bool >( 0, "happy" );
                EXPECT_TRUE( bval );

                int answer = co_await params.get_value< int >(
                    co_await params.get_child( 0, "answer" ), "everything" );
                EXPECT_EQ( answer, 42 );

                testing::node_id        list_node = co_await params.get_child( 0, "list" );
                testing::node_type_enum t2        = co_await params.get_type( list_node );
                EXPECT_EQ( t2, CONTIGUOUS_TREE_ARRAY );

                std::size_t count = co_await params.get_child_count( list_node );
                EXPECT_EQ( count, 3 );

                int lval = co_await params.get_value< int >( list_node, 2 );
                EXPECT_EQ( lval, 2 );

                checkpoint_reached = true;

                co_await params.get_value< int >( 0, "key_not_present" );

                end_reached = true;
        }
};

TEST( params, base )
{
        testing::parameters*        col_ptr;
        testing::parameters_server* server_ptr;

        auto col_send_f = [&]( auto, auto data ) {
                server_ptr->on_msg( data );
        };
        auto server_send_f = [&]( auto, auto data ) {
                col_ptr->on_msg( data );
        };

        testing::parameters coll{ 0, col_send_f };
        col_ptr = &coll;

        nlohmann::json jsn{
            { "pi", 3.141 },
            { "happy", true },
            { "name", "Niels" },
            { "answer", { { "everything", 42 } } },
            { "list", { 1, 0, 2 } },
            { "object", { { "currency", "USD" }, { "value", 42.99 } } } };

        std::optional< testing::data_tree > opt_data =
            testing::json_to_data_tree( pmr::new_delete_resource(), jsn );
        EXPECT_TRUE( opt_data );
        testing::parameters_server server{ 0, std::move( *opt_data ), server_send_f };
        server_ptr = &server;

        params_test_fixture tf{ coll };

        testing::executor exec{ 0u, pmr::new_delete_resource(), tf };

        while ( !exec.finished() ) {
                exec.tick();
        }

        EXPECT_TRUE( tf.checkpoint_reached );
        EXPECT_FALSE( tf.end_reached );
}

}  // namespace emlabcpp
