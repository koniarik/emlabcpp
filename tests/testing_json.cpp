
#include "emlabcpp/experimental/testing/json.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

TEST( TestingJson, json )
{
        nlohmann::json j = {
            { "v1", 42 },
            { "v2", -42 },
            { "v3", false },
            { "v4", 3.1415f },
            { "v5", "wololo" },
            { "v6", nlohmann::json::array( { 1, 2, 3, 4, 5 } ) } };

        pool_dynamic_resource mem_pool;

        std::optional< testing_tree > opt_tree = json_to_testing_tree( &mem_pool, j );

        EXPECT_TRUE( opt_tree );

        nlohmann::json result = testing_tree_to_json( *opt_tree );

        EXPECT_EQ( j, result );
}
