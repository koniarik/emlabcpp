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
                return string_buffer( j.get< std::string >() );
        default:
                break;
        }
        return std::nullopt;
}

nlohmann::json value_type_to_json( const value_type& tv )
{
        nlohmann::json res = match(
            tv,
            []( const string_buffer& buff ) {
                    return nlohmann::json{ std::string_view{ buff } };
            },
            []( const auto& item ) {
                    return nlohmann::json{ item };
            } );

        return res[0];
}

key_type json_to_key_type( const nlohmann::json& j )
{
        return j.get< std::string_view >();
}

std::optional< data_tree >
json_to_data_tree( pmr::memory_resource& mem_res, const nlohmann::json& inpt )
{
        data_tree tree{ mem_res };

        static_function< std::optional< node_id >( const nlohmann::json& j ), 32 > f =
            [&tree, &f]( const nlohmann::json& j ) -> std::optional< node_id > {
                if ( j.is_object() ) {
                        std::optional opt_res = tree.make_object_node();
                        if ( !opt_res )
                                return std::nullopt;
                        auto [nid, oh] = *opt_res;
                        for ( const auto& [key, value] : j.items() ) {
                                std::optional< node_id > chid = f( value );
                                if ( !chid )
                                        return std::nullopt;
                                oh.set( json_to_key_type( key ), *chid );
                        }
                        return nid;
                }
                if ( j.is_array() ) {
                        std::optional opt_res = tree.make_array_node();
                        if ( !opt_res )
                                return std::nullopt;
                        auto [nid, ah] = *opt_res;
                        for ( const nlohmann::json& jj : j ) {
                                std::optional< node_id > chid = f( jj );
                                if ( !chid )
                                        return std::nullopt;
                                ah.append( *chid );
                        }
                        return nid;
                }
                std::optional< value_type > opt_val = json_to_value_type( j );
                if ( !opt_val )
                        return std::nullopt;
                std::optional opt_id = tree.make_value_node( *opt_val );
                if ( !opt_id )
                        return std::nullopt;
                return *opt_id;
        };

        const auto opt_root_id = f( inpt );
        if ( opt_root_id )
                return tree;
        else
                return std::nullopt;
}

nlohmann::json data_tree_to_json( const data_tree& tree )
{
        static_function< nlohmann::json( node_id ), 32 > f =
            [&tree, &f]( const node_id nid ) -> nlohmann::json {
                const auto* const node_ptr = tree.get_node( nid );

                if ( node_ptr == nullptr )
                        return {};

                return match(
                    node_ptr->get_container_handle(),
                    []( const value_type& val ) {
                            return value_type_to_json( val );
                    },
                    [&f]( const data_const_object_handle oh ) {
                            nlohmann::json j = nlohmann::json::object();
                            for ( const auto& [key, chid] : oh ) {
                                    const std::string k{ std::string_view{ key } };
                                    j[k] = f( chid );
                            }
                            return j;
                    },
                    [&f]( const data_const_array_handle ah ) {
                            nlohmann::json j = nlohmann::json::array();
                            for ( const auto& [i, chid] : ah )
                                    j.push_back( f( chid ) );
                            return j;
                    } );
        };

        return f( 0 );
}

}  // namespace emlabcpp::testing
#endif
