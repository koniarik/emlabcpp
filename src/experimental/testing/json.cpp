#include "emlabcpp/experimental/testing/json.h"

#ifdef EMLABCPP_USE_NLOHMANN_JSON

namespace emlabcpp::testing
{

value_type json_to_value_type( const nlohmann::json& j )
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
                        // TODO: might wanna improve this
                        EMLABCPP_LOG( "Got type of json we can't process: " << j.type() );
                        throw std::exception{};
        }
}

nlohmann::json value_type_to_json( const value_type& tv )
{
        nlohmann::json res = match(
            tv,
            [&]( const string_buffer& buff ) {
                    return nlohmann::json{ std::string_view{ buff.begin(), buff.size() } };
            },
            [&]( const auto& item ) {
                    return nlohmann::json{ item };
            } )[0];

        return res;
}

key_type json_to_key_type( const nlohmann::json& j )
{
        return key_type_to_buffer( j.get< std::string_view >() );
}

std::optional< data_tree >
json_to_data_tree( pool_interface* mem_pool, const nlohmann::json& inpt )
{
        data_tree tree{ mem_pool };

        static_function< node_id( const nlohmann::json& j ), 32 > f =
            [&]( const nlohmann::json& j ) -> node_id {
                if ( j.is_object() ) {
                        std::optional opt_res = tree.make_object_node();
                        if ( !opt_res ) {
                                // TODO: might wanna improve this
                                throw std::exception{};
                        }
                        auto [nid, oh] = *opt_res;
                        for ( const auto& [key, value] : j.items() ) {
                                node_id chid = f( value );
                                oh.set( json_to_key_type( key ), chid );
                        }
                        return nid;
                }
                if ( j.is_array() ) {
                        std::optional opt_res = tree.make_array_node();
                        if ( !opt_res ) {
                                // TODO: might wanna improve this
                                throw std::exception{};
                        }
                        auto [nid, ah] = *opt_res;
                        for ( const nlohmann::json& jj : j ) {
                                node_id chid = f( jj );
                                ah.append( chid );
                        }
                        return nid;
                }
                std::optional opt_id = tree.make_value_node( json_to_value_type( j ) );
                if ( !opt_id ) {
                        throw std::exception{};
                }
                return *opt_id;
        };

        try {
                f( inpt );
                return tree;
        }
        catch ( const std::exception& e ) {
                EMLABCPP_LOG( "Build of testing tree failed because " << e.what() );
                return std::nullopt;
        }
}

nlohmann::json data_tree_to_json( const data_tree& tree )
{
        static_function< nlohmann::json( node_id ), 32 > f = [&]( node_id nid ) -> nlohmann::json {
                const auto* node_ptr = tree.get_node( nid );

                if ( node_ptr == nullptr ) {
                        return {};
                }

                return match(
                    node_ptr->get_container_handle(),
                    [&]( const value_type& val ) {
                            return value_type_to_json( val );
                    },
                    [&]( data_const_object_handle oh ) {
                            nlohmann::json j;
                            for ( const auto& [key, chid] : oh ) {
                                    std::string k{ key.begin(), key.size() };
                                    j[k] = f( chid );
                            }
                            return j;
                    },
                    [&]( data_const_array_handle ah ) {
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
