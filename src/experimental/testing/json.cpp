///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
/// and associated documentation files (the "Software"), to deal in the Software without
/// restriction, including without limitation the rights to use, copy, modify, merge, publish,
/// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or
/// substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
/// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
/// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#include "emlabcpp/experimental/testing/json.h"

#include "emlabcpp/experimental/logging.h"
#include "emlabcpp/experimental/testing/base.h"
#include "emlabcpp/match.h"
#include "emlabcpp/pmr/memory_resource.h"
#include "emlabcpp/static_function.h"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#ifdef EMLABCPP_USE_NLOHMANN_JSON

#include <nlohmann/json.hpp>

namespace emlabcpp::testing
{

std::optional< value_type > json_to_value_type( nlohmann::json const& j )
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
                return string_buffer( j.get< std::string >() );
        default:
                break;
        }
        EMLABCPP_ERROR_LOG( "Got type of json we can't process: ", j.type() );
        return std::nullopt;
}

nlohmann::json value_type_to_json( value_type const& tv )
{
        nlohmann::json res = match(
            tv,
            []( string_buffer const& buff ) {
                    return nlohmann::json{ std::string_view{ buff } };
            },
            []( auto const& item ) {
                    return nlohmann::json{ item };
            } );

        return res[0];
}

key_type json_to_key_type( nlohmann::json const& j )
{
        return j.get< std::string_view >();
}

std::optional< data_tree >
json_to_data_tree( pmr::memory_resource& mem_res, nlohmann::json const& inpt )
{
        data_tree tree{ mem_res };

        static_function< std::optional< node_id >( nlohmann::json const& j ), 32 > f =
            [&tree, &f]( nlohmann::json const& j ) -> std::optional< node_id > {
                if ( j.is_object() ) {
                        std::optional opt_res = tree.make_object_node();
                        if ( !opt_res ) {
                                EMLABCPP_ERROR_LOG(
                                    "Failed to build tree object in json conversion" );
                                return std::nullopt;
                        }
                        auto [nid, oh] = *opt_res;
                        for ( auto const& [key, value] : j.items() ) {
                                std::optional< node_id > chid = f( value );
                                if ( !chid )
                                        return std::nullopt;
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
                        for ( nlohmann::json const& jj : j ) {
                                std::optional< node_id > chid = f( jj );
                                if ( !chid )
                                        return std::nullopt;
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

        auto const opt_root_id = f( inpt );
        if ( opt_root_id ) {
                return tree;
        } else {
                EMLABCPP_ERROR_LOG( "Failed to convert testing tree from json" );
                return std::nullopt;
        }
}

nlohmann::json data_tree_to_json( data_tree const& tree )
{
        static_function< nlohmann::json( node_id ), 32 > f =
            [&tree, &f]( node_id const nid ) -> nlohmann::json {
                auto const* const node_ptr = tree.get_node( nid );

                if ( node_ptr == nullptr )
                        return {};

                return match(
                    node_ptr->get_container_handle(),
                    []( value_type const& val ) {
                            return value_type_to_json( val );
                    },
                    [&f]( data_const_object_handle const oh ) {
                            nlohmann::json j = nlohmann::json::object();
                            for ( auto const& [key, chid] : oh ) {
                                    std::string const k{ std::string_view{ key } };
                                    j[k] = f( chid );
                            }
                            return j;
                    },
                    [&f]( data_const_array_handle const ah ) {
                            nlohmann::json j = nlohmann::json::array();
                            for ( auto const& [i, chid] : ah )
                                    j.push_back( f( chid ) );
                            return j;
                    } );
        };

        return f( 0 );
}

}  // namespace emlabcpp::testing
#endif
