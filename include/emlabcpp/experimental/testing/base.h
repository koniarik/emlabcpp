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

#pragma once

namespace emlabcpp::testing
{

using name_buffer     = static_vector< char, 32 >;
using key_type_buffer = static_vector< char, 16 >;

using node_id        = uint32_t;
using child_count    = uint32_t;
using child_id       = uint32_t;
using node_type_enum = contiguous_tree_type_enum;
using key_type       = key_type_buffer;
using string_buffer  = static_vector< char, 32 >;

// TODO: this breaks stuff as it has nlohmann::json serialization overload which is _not a good
// idea_
using value_type = std::variant< int64_t, float, bool, string_buffer >;
static_assert( !alternative_of< uint32_t, value_type > );
using collect_value_type       = std::variant< value_type, contiguous_container_type >;
using run_id                   = uint32_t;
using test_id                  = uint16_t;
using data_tree                = contiguous_tree< key_type, value_type >;
using data_array_handle        = typename data_tree::array_handle;
using data_object_handle       = typename data_tree::object_handle;
using data_const_array_handle  = typename data_tree::const_array_handle;
using data_const_object_handle = typename data_tree::const_object_handle;

struct test_info
{
        name_buffer name;
};

/// TODO: maybe make a function in static_vector namespace?
template < typename T >
T string_to_buffer( const std::string_view sview )
{
        T tmp;
        std::copy_n(
            sview.begin(), std::min( sview.size(), T::capacity ), std::back_inserter( tmp ) );
        return tmp;
}

inline name_buffer name_to_buffer( const std::string_view sview )
{
        return string_to_buffer< name_buffer >( sview );
}

inline key_type_buffer key_type_to_buffer( const std::string_view key )
{
        return string_to_buffer< key_type_buffer >( key );
}

inline string_buffer string_to_buffer( const std::string_view st )
{
        return string_to_buffer< string_buffer >( st );
}

class reactor_interface_adapter;

struct test_result
{
        test_id   tid;
        run_id    rid;
        data_tree collected;
        bool      failed  = false;
        bool      errored = false;

        test_result( const test_id ttid, const run_id trid, pool_interface* const mem_pool )
          : tid( ttid )
          , rid( trid )
          , collected( mem_pool )
        {
        }

        test_result( const test_result& )            = delete;
        test_result& operator=( const test_result& ) = delete;

        test_result( test_result&& ) noexcept            = default;
        test_result& operator=( test_result&& ) noexcept = default;
};

}  // namespace emlabcpp::testing
