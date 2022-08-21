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
#include "emlabcpp/experimental/testing/convert.h"
#include "emlabcpp/experimental/testing/protocol.h"
#include "emlabcpp/match.h"
#include "emlabcpp/static_vector.h"

#pragma once

namespace emlabcpp::testing
{
class testing_record
{
        testing_test_id                    tid_;
        run_id                     rid_;
        testing_reactor_interface_adapter& comm_;
        bool                               errored_ = false;

public:
        testing_record(
            testing_test_id                    tid,
            run_id                     rid,
            testing_reactor_interface_adapter& comm )
          : tid_( tid )
          , rid_( rid )
          , comm_( comm )
        {
        }

        std::optional< testing_node_type > get_param_type( node_id );

        template < typename T >
        std::optional< T > get_param( node_id node )
        {
                std::optional opt_var = get_param_value( node );
                if ( !opt_var ) {
                        EMLABCPP_LOG(
                            "Failed to get param " << node << " with type "
                                                   << pretty_type_name< T >() );
                        return std::nullopt;
                }
                return extract_param< T >( *opt_var, node );
        }

        template < typename T, typename Key >
        std::optional< T > get_param( node_id node, const Key& k )
        {
                std::optional< node_id > opt_nid = get_param_child( node, k );
                if ( !opt_nid ) {
                        EMLABCPP_LOG(
                            "Failed to get param " << k << " of node " << node << " with type "
                                                   << pretty_type_name< T >() );
                        return std::nullopt;
                }
                return get_param< T >( *opt_nid );
        }

        template < typename T, typename Key >
        std::optional< T > get_param( std::optional< node_id > node, const Key& k )
        {
                if ( !node ) {
                        return std::nullopt;
                }
                return get_param< T >( *node, k );
        }

        std::optional< value_type > get_param_value( node_id param );

        std::optional< node_id > get_param_child( node_id, testing_child_id );
        std::optional< node_id > get_param_child( node_id, const key_type& key );
        std::optional< node_id > get_param_child( node_id, std::string_view key );

        std::optional< testing_child_count >
            get_param_child_count( std::optional< node_id > );
        std::optional< testing_child_count > get_param_child_count( node_id nid )
        {
                return get_param_child_count( std::optional{ nid } );
        }

        std::optional< key_type > get_param_key( node_id, testing_child_id );

        bool errored()
        {
                return errored_;
        }

        std::optional< node_id > collect(
            node_id                     parent,
            const std::optional< key_type >& key,
            const testing_collect_arg&          arg );

        std::optional< node_id >
        collect( node_id parent, const testing_collect_arg& arg );

        std::optional< node_id >
        collect( node_id parent, std::string_view k, contiguous_container_type t )
        {
                return collect( parent, convert_key( k ), testing_collect_arg{ t } );
        }
        std::optional< node_id >
        collect( node_id parent, contiguous_container_type t )
        {
                return collect( parent, testing_collect_arg{ t } );
        }

        template < typename Arg >
        std::optional< node_id >
        collect( node_id parent, std::string_view k, const Arg& arg )
        {
                return collect(
                    parent, convert_key( k ), value_type_converter< Arg >::to_value( arg ) );
        }

        template < typename Arg >
        std::optional< node_id > collect( node_id parent, const Arg& arg )
        {
                return collect( parent, value_type_converter< Arg >::to_value( arg ) );
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
        void report_wrong_type_error( node_id, const value_type& var );

        template < typename T >
        std::optional< T > extract_param( const value_type& var, node_id nid )
        {
                std::optional< T > res = value_type_converter< T >::from_value( var );
                if ( !res ) {
                        EMLABCPP_LOG(
                            "Can't extract arg " << pretty_type_name< T >() << " from node " << nid
                                                 << " it has type index: " << var.index() );
                        report_wrong_type_error( nid, var );
                        return std::nullopt;
                }
                return res;
        }

        std::optional< key_type > convert_key( std::string_view sview )
        {
                return key_type_to_buffer( sview );
        }

        template < typename T >
        std::optional< T > read_variant_alternative( node_id );

        template < typename ResultType, auto ID, typename... Args >
        std::optional< ResultType >
        exchange( std::optional< node_id > nid, const Args&... args );
};

}  // namespace emlabcpp::testing
