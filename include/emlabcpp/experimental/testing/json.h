#include "emlabcpp/experimental/testing/base.h"

#ifdef EMLABCPP_USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>

namespace emlabcpp::testing
{

testing_value json_to_testing_value( const nlohmann::json& j );

nlohmann::json testing_value_to_json( const testing_value& tv );

testing_key json_to_testing_key( const nlohmann::json& j );

std::optional< testing_tree >
json_to_testing_tree( pool_interface* mem_pool, const nlohmann::json& inpt );

nlohmann::json testing_tree_to_json( const testing_tree& tree );

}  // namespace emlabcpp::testing

#endif
