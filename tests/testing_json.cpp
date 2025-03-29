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

#include "emlabcpp/experimental/testing/json.h"
#include "emlabcpp/pmr/new_delete_resource.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

TEST( TestingJson, json )
{
        nlohmann::json const j = {
            { "v1", 42 },
            { "v2", -42 },
            { "v3", false },
            { "v4", 3.1415f },
            { "v5", "wololo" },
            { "v6", nlohmann::json::array( { 1, 2, 3, 4, 5 } ) } };

        std::optional< testing::data_tree > opt_tree =
            testing::json_to_data_tree( pmr::new_delete_resource(), j );

        EXPECT_TRUE( opt_tree );

        nlohmann::json const result = testing::data_tree_to_json( *opt_tree );

        EXPECT_EQ( j, result );
}

}  // namespace emlabcpp
