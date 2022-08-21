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

#include "experimental/testing/reactor_interface_adapter.h"

namespace emlabcpp::testing
{

std::optional< testing_node_type > record::get_param_type( node_id nid )
{
        std::optional reply = exchange< param_type_reply, TESTING_PARAM_TYPE >( nid );
        if ( reply ) {
                return reply->type;
        }
        return std::nullopt;
}

std::optional< value_type > record::get_param_value( node_id nid )
{
        std::optional reply = exchange< param_value_reply, TESTING_PARAM_VALUE >( nid );

        if ( reply ) {
                return reply->value;
        }

        return std::nullopt;
}

std::optional< node_id >
record::collect( node_id parent, const testing_collect_arg& arg )
{
        return collect( parent, std::optional< key_type >{}, arg );
}

std::optional< node_id > record::collect(
    node_id                     parent,
    const std::optional< key_type >& key,
    const testing_collect_arg&          arg )
{
        std::optional reply =
            exchange< collect_reply, TESTING_COLLECT >( parent, key, arg );
        if ( reply ) {
                return reply->nid;
        }
        return std::nullopt;
}

std::optional< node_id >
record::get_param_child( node_id nid, child_id chid )
{
        std::optional reply = exchange< param_child_reply, TESTING_PARAM_CHILD >(
            nid, std::variant< key_type, child_id >{ chid } );
        if ( reply ) {
                return reply->chid;
        }
        return std::nullopt;
}

std::optional< node_id >
record::get_param_child( node_id nid, std::string_view key )
{
        return get_param_child( nid, key_type_to_buffer( key ) );
}

std::optional< node_id >
record::get_param_child( node_id nid, const key_type& key )
{
        // TODO duplication of other overload /o\...
        std::optional reply = exchange< param_child_reply, TESTING_PARAM_CHILD >(
            nid, std::variant< key_type, child_id >{ key } );
        if ( reply ) {
                return reply->chid;
        }
        return std::nullopt;
}

std::optional< child_count >
record::get_param_child_count( std::optional< node_id > nid )
{
        std::optional reply =
            exchange< param_child_count_reply, TESTING_PARAM_CHILD_COUNT >( nid );
        if ( reply ) {
                return reply->count;
        }
        return std::nullopt;
}

std::optional< key_type >
record::get_param_key( node_id nid, child_id chid )
{
        std::optional reply = exchange< param_key_reply, TESTING_PARAM_KEY >( nid, chid );
        if ( reply ) {
                return reply->key;
        }
        return std::nullopt;
}

void record::report_wrong_type_error( node_id nid, const value_type& )
{
        comm_.report_failure< TESTING_WRONG_TYPE_E >( nid );
}

template < typename T >
std::optional< T > record::read_variant_alternative( node_id )
{
        std::optional< controller_reactor_variant > opt_var = comm_.read_variant();
        if ( !opt_var ) {
                comm_.report_failure< TESTING_NO_RESPONSE_E >( T::tag );
                return {};
        }

        const T* val_ptr = std::get_if< T >( &*opt_var );
        if ( val_ptr != nullptr ) {
                return *val_ptr;
        }

        const auto* tree_err = std::get_if< tree_error_reply >( &*opt_var );
        if ( tree_err ) {
                comm_.report_failure< TESTING_TREE_E >( tree_err->nid, tree_err->err );
        }

        comm_.report_failure< TESTING_WRONG_MESSAGE_E >();

        return std::nullopt;
}

template < typename ResultType, auto ID, typename... Args >
std::optional< ResultType >
record::exchange( std::optional< node_id > opt_nid, const Args&... args )
{
        if ( !opt_nid ) {
                return std::nullopt;
        }

        node_id nid = *opt_nid;

        comm_.reply< ID >( rid_, nid, args... );

        return read_variant_alternative< ResultType >( nid );
}

}  // namespace emlabcpp::testing
