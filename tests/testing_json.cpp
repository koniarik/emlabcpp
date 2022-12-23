#include "emlabcpp/experimental/testing/json.h"
#include "emlabcpp/pmr/new_delete_resource.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

TEST( TestingJson, json )
{
        nlohmann::json j = {
            { "v1", 42 },
            { "v2", -42 },
            { "v3", false },
            { "v4", 3.1415f },
            { "v5", "wololo" },
            { "v6", nlohmann::json::array( { 1, 2, 3, 4, 5 } ) } };

        std::optional< testing::data_tree > opt_tree =
            testing::json_to_data_tree( pmr::new_delete_resource(), j );

        EXPECT_TRUE( opt_tree );

        nlohmann::json result = testing::data_tree_to_json( *opt_tree );

        EXPECT_EQ( j, result );
}

}  // namespace emlabcpp
