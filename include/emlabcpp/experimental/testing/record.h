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
#include "emlabcpp/experimental/testing/base.h"
#include "emlabcpp/experimental/testing/protocol.h"
#include "emlabcpp/match.h"
#include "emlabcpp/static_vector.h"

#pragma once

namespace emlabcpp
{
class testing_record
{
        testing_test_id                    tid_;
        testing_run_id                     rid_;
        testing_reactor_interface_adapter& comm_;
        bool                               errored_ = false;

public:
        testing_record(
            testing_test_id                    tid,
            testing_run_id                     rid,
            testing_reactor_interface_adapter& comm )
          : tid_( tid )
          , rid_( rid )
          , comm_( comm )
        {
        }

        std::optional< testing_node_type > get_param_type( testing_node_id );

        template < typename T >
        std::optional< T > get_param( testing_node_id node )
        {
                std::optional opt_var = get_param_value( node );
                if ( !opt_var ) {
                        return std::nullopt;
                }
                return extract_arg< T >( *opt_var, node );
        }

        template < typename T, typename Key >
        std::optional< T > get_param( testing_node_id node, const Key& k )
        {
                std::optional< testing_node_id > opt_nid = get_param_child( node, k );
                if ( !opt_nid ) {
                        return std::nullopt;
                }
                return get_param< T >( *opt_nid );
        }

        template < typename T, typename Key >
        std::optional< T > get_param( std::optional< testing_node_id > node, const Key& k )
        {
                if ( !node ) {
                        return std::nullopt;
                }
                return get_param< T >( *node, k );
        }

        std::optional< testing_value > get_param_value( testing_node_id param );

        std::optional< testing_node_id > get_param_child( testing_node_id, testing_child_id );
        std::optional< testing_node_id > get_param_child( testing_node_id, const testing_key& key );
        std::optional< testing_node_id > get_param_child( testing_node_id, std::string_view key );

        std::optional< testing_child_count > get_param_child_count( testing_node_id );

        std::optional< testing_key > get_param_key( testing_node_id, testing_child_id );

        bool errored()
        {
                return errored_;
        }

        std::optional< testing_node_id > collect(
            testing_node_id                     parent,
            const std::optional< testing_key >& key,
            const testing_collect_arg&          arg );

        std::optional< testing_node_id >
        collect( testing_node_id parent, const testing_collect_arg& arg );

        template < typename Key, typename Arg >
        std::optional< testing_node_id >
        collect( testing_node_id parent, const Key& k, const Arg& arg )
        {
                return collect( parent, convert_key( k ), convert_arg( arg ) );
        }

        template < typename Arg >
        std::optional< testing_node_id > collect( testing_node_id parent, const Arg& arg )
        {
                return collect( parent, convert_arg( arg ) );
        }

        void fail()
        {
                errored_ = true;
        }

        void success()
        {
        }

        void expect( bool val )
        {
                val ? success() : fail();
        }

private:
        void report_wrong_type_error( testing_node_id, const testing_value& var );

        template < typename T >
        std::optional< T > extract_arg( const testing_value& var, testing_node_id nid )
        {
                if ( !std::holds_alternative< T >( var ) ) {
                        report_wrong_type_error( nid, var );
                        return {};
                }
                return std::get< T >( var );
        }

        std::optional< testing_key > convert_key( std::string_view sview )
        {
                return testing_key_to_buffer( sview );
        }

        testing_collect_arg convert_arg( const alternative_of< testing_value > auto& arg )
        {
                return testing_value{ arg };
        }

        testing_collect_arg convert_arg( std::string_view arg )
        {
                return testing_value{ testing_string_to_buffer( arg ) };
        }

        testing_collect_arg convert_arg( contiguous_container_type arg )
        {
                return arg;
        }

        std::optional< testing_controller_reactor_variant >
            read_variant( testing_node_id, testing_messages_enum );

        template < typename ResultType, auto ID, typename... Args >
        std::optional< ResultType > exchange( testing_node_id nid, const Args&... args );
};
}  // namespace emlabcpp
