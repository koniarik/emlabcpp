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

#include "emlabcpp/algorithm.h"
#include "emlabcpp/experimental/contiguous_tree/tree.h"
#include "emlabcpp/experimental/string_buffer.h"
#include "emlabcpp/static_function.h"

#include <algorithm>
#include <map>
#include <memory_resource>
#include <variant>

#pragma once

namespace emlabcpp::testing
{

using name_buffer     = string_buffer< 32 >;
using key_type_buffer = string_buffer< 32 >;

using node_id        = uint32_t;
using child_count    = uint32_t;
using child_id       = uint32_t;
using node_type_enum = contiguous_tree_type;
using string_buffer  = string_buffer< 32 >;
using key_type       = key_type_buffer;

// TODO: this breaks stuff as it has nlohmann::json serialization overload which is _not a good
// idea_
using value_type = std::variant< int64_t, float, bool, string_buffer >;
static_assert( !alternative_of< uint32_t, value_type > );
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

class reactor_interface_adapter;

struct test_result
{
        test_id tid;
        run_id  rid;
        bool    failed  = false;
        bool    errored = false;

        test_result( const test_id ttid, const run_id trid )
          : tid( ttid )
          , rid( trid )
        {
        }

        test_result( const test_result& )            = delete;
        test_result& operator=( const test_result& ) = delete;

        test_result( test_result&& ) noexcept            = default;
        test_result& operator=( test_result&& ) noexcept = default;
};

}  // namespace emlabcpp::testing
