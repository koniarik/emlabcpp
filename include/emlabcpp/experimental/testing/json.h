#include "emlabcpp/experimental/testing/base.h"

#ifdef EMLABCPP_USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>

namespace emlabcpp::testing
{

std::optional< value_type > json_to_value_type( const nlohmann::json& j );

nlohmann::json value_type_to_json( const value_type& tv );

key_type json_to_key_type( const nlohmann::json& j );

std::optional< data_tree >
json_to_data_tree( pool_interface* mem_pool, const nlohmann::json& inpt );

nlohmann::json data_tree_to_json( const data_tree& tree );

}  // namespace emlabcpp::testing

#endif
