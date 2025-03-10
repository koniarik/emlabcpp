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

        [[nodiscard]] std::string_view get_name() const override
        {
                return "wololo";
        }

        testing::coroutine< void > run( pmr::memory_resource& ) override
        {
                testing::node_type_enum t =
                    co_await params.get_type( co_await params.get_child( 0, "pi" ) );
                EXPECT_EQ( t, contiguous_tree_type::VALUE );

                static_assert( alternative_of< bool, testing::value_type > );
                bool bval = co_await params.get_value< bool >( 0, "happy" );
                EXPECT_TRUE( bval );

                int answer = co_await params.get_value< int >(
                    co_await params.get_child( 0, "answer" ), "everything" );
                EXPECT_EQ( answer, 42 );

                testing::node_id        list_node = co_await params.get_child( 0, "list" );
                testing::node_type_enum t2        = co_await params.get_type( list_node );
                EXPECT_EQ( t2, contiguous_tree_type::ARRAY );

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

        auto col_send_f = [&]( auto, auto data ) -> result {
                return server_ptr->on_msg( data ).has_errored_result();
        };
        auto server_send_f = [&]( auto, auto data ) -> result {
                return col_ptr->on_msg( data ).has_errored_result();
        };

        testing::parameters coll{ 0, col_send_f };
        col_ptr = &coll;

        const nlohmann::json jsn{
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

        while ( !exec.finished() )
                exec.tick();

        EXPECT_TRUE( tf.checkpoint_reached );
        EXPECT_FALSE( tf.end_reached );
}

}  // namespace emlabcpp
