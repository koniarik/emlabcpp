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

using namespace emlabcpp;

std::optional< testing_node_type > testing_record::get_param_type( testing_node_id nid )
{
        return exchange< testing_node_type, TESTING_PARAM_TYPE >( nid );
}

std::optional< testing_value > testing_record::get_param_value( testing_node_id nid )
{
        comm_.reply< TESTING_PARAM_VALUE >( rid_, nid );

        std::optional< testing_value >                      res;
        std::optional< testing_controller_reactor_variant > opt_variant =
            read_variant( nid, TESTING_PARAM_VALUE );
        if ( !opt_variant ) {
                return {};
        }
        apply_on_match(
            *opt_variant,
            [&]( tag< TESTING_PARAM_VALUE >, testing_run_id, const testing_value& var ) {
                    res = var;
            },
            [&]< auto ID >( tag< ID >, auto... ){} );

        return res;
}

std::optional< testing_node_id >
testing_record::collect( testing_node_id parent, const testing_collect_arg& arg )
{
        return collect( parent, std::optional< testing_key >{}, arg );
}

std::optional< testing_node_id > testing_record::collect(
    testing_node_id                     parent,
    const std::optional< testing_key >& key,
    const testing_collect_arg&          arg )
{
        comm_.reply< TESTING_COLLECT >( rid_, parent, key, arg );
        std::optional< testing_node_id >                    res;
        std::optional< testing_controller_reactor_variant > opt_variant =
            read_variant( parent, TESTING_COLLECT );
        if ( !opt_variant ) {
                return res;
        }
        apply_on_match(
            *opt_variant,
            [&]( tag< TESTING_COLLECT >, testing_run_id, testing_node_id val ) {
                    res = val;
            },
            [&]< auto ID >( tag< ID >, auto... ){} );
        return res;
}

std::optional< testing_node_id >
testing_record::get_param_child( testing_node_id nid, testing_child_id chid )
{
        return exchange< testing_node_id, TESTING_PARAM_CHILD >(
            nid, std::variant< testing_key, testing_child_id >{ chid } );
}

std::optional< testing_node_id >
testing_record::get_param_child( testing_node_id nid, std::string_view key )
{
        return get_param_child( nid, testing_key_to_buffer( key ) );
}

std::optional< testing_node_id >
testing_record::get_param_child( testing_node_id nid, const testing_key& key )
{
        return exchange< testing_node_id, TESTING_PARAM_CHILD >(
            nid, std::variant< testing_key, testing_child_id >{ key } );
}

std::optional< testing_child_count >
testing_record::get_param_child_count( std::optional< testing_node_id > nid )
{
        return exchange< testing_child_count, TESTING_PARAM_CHILD_COUNT >( nid );
}

std::optional< testing_key >
testing_record::get_param_key( testing_node_id nid, testing_child_id chid )
{
        return exchange< testing_key, TESTING_PARAM_KEY >( nid, chid );
}

void testing_record::report_wrong_type_error( testing_node_id nid, const testing_value& )
{
        comm_.report_failure< TESTING_WRONG_TYPE_E >( nid );
}

std::optional< testing_controller_reactor_variant >
testing_record::read_variant( testing_node_id, testing_messages_enum desired )
{
        std::optional< testing_controller_reactor_variant > opt_var = comm_.read_variant();
        if ( !opt_var ) {
                comm_.report_failure< TESTING_NO_RESPONSE_E >( TESTING_PARAM_VALUE );
                return {};
        }

        apply_on_match(
            *opt_var,
            [&]( tag< TESTING_TREE_ERROR >,
                 testing_run_id,
                 contiguous_request_adapter_errors_enum err,
                 testing_node_id                        nid ) {
                    comm_.report_failure< TESTING_TREE_E >( nid, err );
            },
            [&]< auto ID >( tag< ID >, const auto&... ) {
                    if ( ID == desired ) {
                            return;
                    }
                    comm_.report_failure< TESTING_WRONG_MESSAGE_E >( ID );
            } );
        return opt_var;
}

template < typename ResultType, auto ID, typename... Args >
std::optional< ResultType >
testing_record::exchange( std::optional< testing_node_id > opt_nid, const Args&... args )
{
        if ( !opt_nid ) {
                return std::nullopt;
        }

        testing_node_id nid = *opt_nid;

        comm_.reply< ID >( rid_, nid, args... );

        std::optional< testing_controller_reactor_variant > opt_variant = read_variant( nid, ID );
        if ( !opt_variant ) {
                return std::nullopt;
        }
        using proto_tuple = testing_controller_reactor_group::cmd_value_type< ID >;
        if ( std::holds_alternative< proto_tuple >( *opt_variant ) ) {
                auto* tpl = std::get_if< proto_tuple >( &*opt_variant );
                return std::get< 2 >( *tpl );
        }
        return std::nullopt;
}
