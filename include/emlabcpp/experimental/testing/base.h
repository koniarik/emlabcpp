// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//
//  Copyright © 2022 Jan Veverak Koniarik
//  This file is part of project: emlabcpp
//
#include "emlabcpp/algorithm.h"
#include "emlabcpp/allocator/pool.h"
#include "emlabcpp/experimental/contiguous_tree/tree.h"
#include "emlabcpp/static_vector.h"

#include <map>
#include <variant>

#ifdef EMLABCPP_USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

#pragma once

namespace emlabcpp
{

using testing_name_buffer = static_vector< char, 32 >;
using testing_key_buffer  = static_vector< char, 16 >;

using testing_node_id       = uint32_t;
using testing_child_count   = uint32_t;
using testing_child_id      = uint32_t;
using testing_node_type     = contiguous_tree_type_enum;
using testing_key           = testing_key_buffer;
using testing_string_buffer = static_vector< char, 32 >;

// TODO: this breaks stuff as it has nlohmann::json serialization overload which is _not a good
// idea_
using testing_value               = std::variant< uint64_t, int64_t, bool, testing_string_buffer >;
using testing_collect_arg         = std::variant< testing_value, contiguous_container_type >;
using testing_run_id              = uint32_t;
using testing_test_id             = uint16_t;
using testing_tree                = contiguous_tree< testing_key, testing_value >;
using testing_node                = typename testing_tree::node_type;
using testing_array_handle        = typename testing_tree::array_handle;
using testing_object_handle       = typename testing_tree::object_handle;
using testing_const_array_handle  = typename testing_tree::const_array_handle;
using testing_const_object_handle = typename testing_tree::const_object_handle;

struct test_info
{
        testing_name_buffer name;
};

/// TODO: maybe make a function in static_vector namespace?
template < typename T >
inline T testing_string_to_buffer( std::string_view sview )
{
        T tmp;
        std::copy_n(
            sview.begin(), std::min( sview.size(), T::capacity ), std::back_inserter( tmp ) );
        return tmp;
}

inline testing_name_buffer testing_name_to_buffer( std::string_view sview )
{
        return testing_string_to_buffer< testing_name_buffer >( sview );
}

inline testing_key_buffer testing_key_to_buffer( std::string_view key )
{
        return testing_string_to_buffer< testing_key_buffer >( key );
}

inline testing_string_buffer testing_string_to_buffer( std::string_view st )
{
        return testing_string_to_buffer< testing_string_buffer >( st );
}

class testing_reactor_interface_adapter;

struct testing_result
{
        testing_test_id tid;
        testing_run_id  rid;
        testing_tree    collected;
        bool            failed  = false;
        bool            errored = false;

        testing_result( testing_test_id ttid, testing_run_id trid, pool_interface* mem_pool )
          : tid( ttid )
          , rid( trid )
          , collected( mem_pool )
        {
        }

        testing_result( const testing_result& )            = delete;
        testing_result& operator=( const testing_result& ) = delete;

        testing_result( testing_result&& ) noexcept            = default;
        testing_result& operator=( testing_result&& ) noexcept = default;
};

#ifdef EMLABCPP_USE_NLOHMANN_JSON

inline emlabcpp::testing_value json_to_testing_value( const nlohmann::json& j )
{
        using value_t = nlohmann::json::value_t;

        switch ( j.type() ) {
                case value_t::boolean:
                        return j.get< bool >();
                case value_t::number_integer:
                        return j.get< int64_t >();
                case value_t::number_unsigned:
                        return j.get< uint64_t >();
                case value_t::string:
                        return testing_string_to_buffer( j.get< std::string >() );
                default:
                        // TODO: might wanna improve this
                        throw std::exception{};
        }
}

inline emlabcpp::testing_key json_to_testing_key( const nlohmann::json& j )
{
        return testing_key_to_buffer( j.get< std::string_view >() );
}

inline std::optional< testing_tree >
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
        catch ( std::exception& ) {
                return std::nullopt;
        }
}

#endif

}  // namespace emlabcpp

