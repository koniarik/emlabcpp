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
#include "emlabcpp/experimental/testing/record.h"

namespace emlabcpp::testing
{

param_type_awaiter record::get_param_type( const node_id nid )
{
        return param_type_awaiter{ param_type_request{ rid_, nid }, &comm_ };
}

collect_awaiter
record::collect( const node_id parent, const collect_value_type& arg, const bool expects_reply )
{
        return collect( parent, std::optional< key_type >{}, arg, expects_reply );
}

collect_awaiter record::collect(
    const node_id                   parent,
    const std::string_view          k,
    const contiguous_container_type t,
    const bool                      expects_reply )
{
        return collect( parent, convert_key( k ), collect_value_type{ t }, expects_reply );
}

collect_awaiter
record::collect( const node_id parent, const contiguous_container_type t, const bool expects_reply )
{
        return collect( parent, collect_value_type{ t }, expects_reply );
}

collect_awaiter record::collect(
    const node_id                    parent,
    const std::optional< key_type >& key,
    const collect_value_type&        arg,
    const bool                       expects_reply )
{
        return collect_awaiter{
            collect_request{
                .rid           = rid_,
                .parent        = parent,
                .expects_reply = expects_reply,
                .opt_key       = key,
                .value         = arg },
            &comm_ };
}

param_child_awaiter record::get_param_child( const node_id nid, const child_id chid )
{
        return param_child_awaiter{ param_child_request{ rid_, nid, chid }, &comm_ };
}

param_child_awaiter record::get_param_child( const node_id nid, const std::string_view key )
{
        return get_param_child( nid, key_type_to_buffer( key ) );
}

param_child_awaiter record::get_param_child( const node_id nid, const key_type& key )
{
        return param_child_awaiter{ param_child_request{ rid_, nid, key }, &comm_ };
}

param_child_count_awaiter record::get_param_child_count( const std::optional< node_id > nid )
{
        return param_child_count_awaiter{
            param_child_count_request{ .rid = rid_, .parent = *nid }, &comm_ };
}

param_key_awaiter record::get_param_key( const node_id nid, const child_id chid )
{
        return param_key_awaiter{ param_key_request{ rid_, nid, chid }, &comm_ };
}

}  // namespace emlabcpp::testing
