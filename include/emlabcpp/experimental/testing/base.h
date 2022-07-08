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
#include "emlabcpp/static_vector.h"

#include <map>
#include <variant>

#pragma once

namespace emlabcpp
{

using testing_key_buffer  = static_vector< char, 16 >;
using testing_name_buffer = static_vector< char, 32 >;

using testing_node_id       = uint32_t;
using testing_key           = std::variant< uint32_t, testing_key_buffer >;
using testing_string_buffer = static_vector< char, 32 >;
using testing_arg_variant =
    std::variant< std::monostate, uint64_t, int64_t, bool, testing_string_buffer >;
using testing_run_id  = uint32_t;
using testing_test_id = uint16_t;

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

struct testing_data_node
{
        testing_node_id                id;
        testing_key                    key;
        testing_arg_variant            var;
        pool_list< testing_data_node > children;

        testing_data_node(
            testing_node_id            id,
            const testing_key&         k,
            const testing_arg_variant& var,
            pool_interface*            mem_pool )
          : id( id )
          , key( k )
          , var( var )
          , children( mem_pool )
        {
        }

        testing_data_node( const testing_data_node& )            = delete;
        testing_data_node& operator=( const testing_data_node& ) = delete;

        testing_data_node( testing_data_node&& ) noexcept            = default;
        testing_data_node& operator=( testing_data_node&& ) noexcept = default;

        void add_child( testing_node_id id, const testing_key& k, const testing_arg_variant& var )
        {
                children.emplace_back( id, k, var, children.get_allocator().get_resource() );
        }

        testing_data_node* find_node( testing_node_id nid )
        {
                if ( id == nid ) {
                        return this;
                }
                auto iter = find_if( children, [&]( testing_data_node& child ) {
                        return child.find_node( nid );
                } );
                if ( iter == children.end() ) {
                        return nullptr;
                }
                return &*iter;
        }
};

struct testing_result
{
        testing_test_id   tid;
        testing_run_id    rid;
        testing_data_node data_root;
        bool              failed  = false;
        bool              errored = false;

        testing_result( testing_test_id ttid, testing_run_id trid, pool_interface* mem_pool )
          : tid( ttid )
          , rid( trid )
          , data_root( 0, testing_key_to_buffer( "root" ), std::monostate{}, mem_pool )
        {
        }

        testing_result( const testing_result& )            = delete;
        testing_result& operator=( const testing_result& ) = delete;

        testing_result( testing_result&& ) noexcept            = default;
        testing_result& operator=( testing_result&& ) noexcept = default;
};

class testing_reactor_interface_adapter;

}  // namespace emlabcpp
