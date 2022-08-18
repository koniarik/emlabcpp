#include "emlabcpp/experimental/testing/json.h"

#ifdef EMLABCPP_USE_NLOHMANN_JSON

namespace emlabcpp
{

emlabcpp::testing_value json_to_testing_value( const nlohmann::json& j )
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
                        return testing_string_to_buffer( j.get< std::string >() );
                default:
                        // TODO: might wanna improve this
                        EMLABCPP_LOG( "Got type of json we can't process: " << j.type() );
                        throw std::exception{};
        }
}

nlohmann::json testing_value_to_json( const testing_value& tv )
{
        nlohmann::json res = match(
            tv,
            [&]( const testing_string_buffer& buff ) {
                    return nlohmann::json{ std::string_view{ buff.begin(), buff.size() } };
            },
            [&]( const auto& item ) {
                    return nlohmann::json{ item };
            } )[0];

        return res;
}

emlabcpp::testing_key json_to_testing_key( const nlohmann::json& j )
{
        return testing_key_to_buffer( j.get< std::string_view >() );
}

std::optional< testing_tree >
json_to_testing_tree( pool_interface* mem_pool, const nlohmann::json& inpt )
{
        testing_tree tree{ mem_pool };

        static_function< testing_node_id( const nlohmann::json& j ), 32 > f =
            [&]( const nlohmann::json& j ) -> testing_node_id {
                if ( j.is_object() ) {
                        std::optional opt_res = tree.make_object_node();
                        if ( !opt_res ) {
                                // TODO: might wanna improve this
                                throw std::exception{};
                        }
                        auto [nid, oh] = *opt_res;
                        for ( const auto& [key, value] : j.items() ) {
                                testing_node_id chid = f( value );
                                oh.set( json_to_testing_key( key ), chid );
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
                                testing_node_id chid = f( jj );
                                ah.append( chid );
                        }
                        return nid;
                }
                std::optional opt_id = tree.make_value_node( json_to_testing_value( j ) );
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

nlohmann::json testing_tree_to_json( const testing_tree& tree )
{
        static_function< nlohmann::json( testing_node_id ), 32 > f =
            [&]( testing_node_id nid ) -> nlohmann::json {
                const testing_node* node_ptr = tree.get_node( nid );

                if ( !node_ptr ) {
                        return {};
                }

                return match(
                    node_ptr->get_container_handle(),
                    [&]( const testing_value& val ) {
                            return testing_value_to_json( val );
                    },
                    [&]( testing_const_object_handle oh ) {
                            nlohmann::json j;
                            for ( const auto& [key, chid] : oh ) {
                                    std::string k{ key.begin(), key.size() };
                                    j[k] = f( chid );
                            }
                            return j;
                    },
                    [&]( testing_const_array_handle ah ) {
                            nlohmann::json j;
                            for ( const auto& [i, chid] : ah ) {
                                    j.push_back( f( chid ) );
                            }
                            return j;
                    } );
        };

        return f( 0 );
}

}  // namespace emlabcpp
#endif
