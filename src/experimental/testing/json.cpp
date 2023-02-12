#include "emlabcpp/experimental/testing/json.h"

#include "emlabcpp/static_function.h"

#ifdef EMLABCPP_USE_NLOHMANN_JSON

namespace emlabcpp::testing
{

std::optional< value_type > json_to_value_type( const nlohmann::json& j )
{
        using value_t = nlohmann::json::value_t;

        switch ( j.type() ) {
        case value_t::boolean:
                return j.get< bool >();
        case value_t::number_integer:
        case value_t::number_unsigned:
                return j.get< int64_t >();
        case value_t::number_float:
                return j.get< float >();
        case value_t::string:
                return string_to_buffer( j.get< std::string >() );
        default:
                break;
        }
        EMLABCPP_ERROR_LOG( "Got type of json we can't process: ", j.type() );
        return std::nullopt;
}

nlohmann::json value_type_to_json( const value_type& tv )
{
        nlohmann::json res = match(
            tv,
            []( const string_buffer& buff ) {
                    return nlohmann::json{ std::string_view{ buff.begin(), buff.size() } };
            },
            []( const auto& item ) {
                    return nlohmann::json{ item };
            } );

        return res[0];
}

key_type json_to_key_type( const nlohmann::json& j )
{
        return key_type_to_buffer( j.get< std::string_view >() );
}

std::optional< data_tree >
json_to_data_tree( pmr::memory_resource& mem_res, const nlohmann::json& inpt )
{
        data_tree tree{ mem_res };

        static_function< std::optional< node_id >( const nlohmann::json& j ), 32 > f =
            [&tree, &f]( const nlohmann::json& j ) -> std::optional< node_id > {
                if ( j.is_object() ) {
                        std::optional opt_res = tree.make_object_node();
                        if ( !opt_res ) {
                                EMLABCPP_ERROR_LOG(
                                    "Failed to build tree object in json conversion" );
                                return std::nullopt;
                        }
                        auto [nid, oh] = *opt_res;
                        for ( const auto& [key, value] : j.items() ) {
                                std::optional< node_id > chid = f( value );
                                if ( !chid ) {
                                        return std::nullopt;
                                }
                                oh.set( json_to_key_type( key ), *chid );
                        }
                        return nid;
                }
                if ( j.is_array() ) {
                        std::optional opt_res = tree.make_array_node();
                        if ( !opt_res ) {
                                EMLABCPP_ERROR_LOG(
                                    "Failed to build tree array in json conversion" );
                                return std::nullopt;
                        }
                        auto [nid, ah] = *opt_res;
                        for ( const nlohmann::json& jj : j ) {
                                std::optional< node_id > chid = f( jj );
                                if ( !chid ) {
                                        return std::nullopt;
                                }
                                ah.append( *chid );
                        }
                        return nid;
                }
                std::optional< value_type > opt_val = json_to_value_type( j );
                if ( !opt_val ) {
                        EMLABCPP_ERROR_LOG( "Failed to convert value in json conversion" );
                        return std::nullopt;
                }
                std::optional opt_id = tree.make_value_node( *opt_val );
                if ( !opt_id ) {
                        EMLABCPP_ERROR_LOG( "Failed to build value node in json conversion" );
                        return std::nullopt;
                }
                return *opt_id;
        };

        const auto opt_root_id = f( inpt );
        if ( opt_root_id ) {
                return tree;
        } else {
                EMLABCPP_ERROR_LOG( "Failed to convert testing tree from json" );
                return std::nullopt;
        }
}

nlohmann::json data_tree_to_json( const data_tree& tree )
{
        static_function< nlohmann::json( node_id ), 32 > f =
            [&tree, &f]( const node_id nid ) -> nlohmann::json {
                const auto* const node_ptr = tree.get_node( nid );

                if ( node_ptr == nullptr ) {
                        return {};
                }

                return match(
                    node_ptr->get_container_handle(),
                    []( const value_type& val ) {
                            return value_type_to_json( val );
                    },
                    [&f]( const data_const_object_handle oh ) {
                            nlohmann::json j;
                            for ( const auto& [key, chid] : oh ) {
                                    const std::string k{ key.begin(), key.size() };
                                    j[k] = f( chid );
                            }
                            return j;
                    },
                    [&f]( const data_const_array_handle ah ) {
                            nlohmann::json j;
                            for ( const auto& [i, chid] : ah ) {
                                    j.push_back( f( chid ) );
                            }
                            return j;
                    } );
        };

        return f( 0 );
}

}  // namespace emlabcpp::testing
#endif
