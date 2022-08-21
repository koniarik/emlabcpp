#include "emlabcpp/experimental/testing/base.h"

#ifdef EMLABCPP_USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>

namespace emlabcpp::testing
{

value_type json_to_value_type( const nlohmann::json& j );

nlohmann::json value_type_to_json( const value_type& tv );

testing_key json_to_testing_key( const nlohmann::json& j );

std::optional< testing_tree >
json_to_testing_tree( pool_interface* mem_pool, const nlohmann::json& inpt );

nlohmann::json testing_tree_to_json( const testing_tree& tree );

}  // namespace emlabcpp::testing

#endif
