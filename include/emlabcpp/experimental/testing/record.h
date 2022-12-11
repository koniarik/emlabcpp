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
#include "emlabcpp/experimental/testing/coroutine.h"
#include "emlabcpp/experimental/testing/protocol.h"
#include "emlabcpp/match.h"
#include "emlabcpp/static_vector.h"

#pragma once

namespace emlabcpp::testing
{
class record
{
        run_id                     rid_;
        reactor_interface_adapter& comm_;
        bool                       errored_ = false;

public:
        record( const test_id, const run_id rid, reactor_interface_adapter& comm )
          : rid_( rid )
          , comm_( comm )
        {
        }

        param_type_awaiter get_param_type( node_id );

        template < typename T >
        param_value_awaiter< T > get_param( const node_id node )
        {
                return param_value_awaiter< T >{ param_value_request{ rid_, node }, &comm_ };
        }

        template < typename T >
        param_value_key_awaiter< T > get_param( const node_id node, const child_id chid )
        {
                return param_value_key_awaiter< T >{
                    param_value_key_request{ rid_, node, chid }, &comm_ };
        }
        template < typename T >
        param_value_key_awaiter< T > get_param( const node_id node, const key_type& k )
        {
                return param_value_key_awaiter< T >{
                    param_value_key_request{ rid_, node, k }, &comm_ };
        }
        template < typename T >
        param_value_key_awaiter< T > get_param( const node_id node, std::string_view k )
        {
                return get_param< T >( node, key_type_to_buffer( k ) );
        }

        param_child_awaiter get_param_child( node_id, child_id );
        param_child_awaiter get_param_child( node_id, const key_type& key );
        param_child_awaiter get_param_child( node_id, std::string_view key );

        param_child_count_awaiter get_param_child_count( std::optional< node_id > );
        param_child_count_awaiter get_param_child_count( const node_id nid )
        {
                return get_param_child_count( std::optional{ nid } );
        }

        param_key_awaiter get_param_key( node_id, child_id );

        bool errored() const
        {
                return errored_;
        }

        collect_awaiter collect(
            node_id                          parent,
            const std::optional< key_type >& key,
            const collect_value_type&        arg,
            bool                             expects_reply = true );

        collect_awaiter
        collect( node_id parent, const collect_value_type& arg, bool expects_reply = true );

        collect_awaiter collect(
            node_id                   parent,
            std::string_view          k,
            contiguous_container_type t,
            bool                      expects_reply = true );

        collect_awaiter
        collect( node_id parent, contiguous_container_type t, bool expects_reply = true );

        template < typename Arg >
        collect_awaiter collect(
            const node_id          parent,
            const std::string_view k,
            const Arg&             arg,
            const bool             expects_reply = true )
        {
                return collect(
                    parent,
                    convert_key( k ),
                    value_type_converter< Arg >::to_value( arg ),
                    expects_reply );
        }

        template < typename Arg >
        collect_awaiter
        collect( const node_id parent, const Arg& arg, const bool expects_reply = true )
        {
                const value_type& val = value_type_converter< Arg >::to_value( arg );
                return collect( parent, collect_value_type{ val }, expects_reply );
        }

        void fail()
        {
                errored_ = true;
        }

        void success() const
        {
        }

        void expect( const bool val )
        {
                val ? success() : fail();
        }

private:
        std::optional< key_type > convert_key( const std::string_view sview )
        {
                return key_type_to_buffer( sview );
        }
};

}  // namespace emlabcpp::testing
