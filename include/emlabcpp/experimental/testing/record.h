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
        testing_node_id                    last_id_ = 0;

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

        template < typename T >
        std::optional< T > get_arg( std::string_view ikey )
        {
                auto          key     = testing_key_to_buffer( ikey );
                std::optional opt_var = get_arg_variant( key );
                if ( !opt_var ) {
                        return {};
                }
                return extract_arg< T >( *opt_var, key );
        }

        template < typename T >
        std::optional< T > get_arg( uint32_t v )
        {
                std::optional opt_var = get_arg_variant( testing_key{ v } );
                if ( !opt_var ) {
                        return {};
                }
                return extract_arg< T >( *opt_var, testing_key{ v } );
        }

        std::optional< testing_arg_variant > get_arg_variant( const testing_key& key );

        bool errored()
        {
                return errored_;
        }

        testing_node_id
        collect( testing_node_id parent, const testing_key& key, const testing_arg_variant& arg );

        template < typename Key, typename Arg >
        testing_node_id collect( testing_node_id parent, const Key& k, const Arg& arg )
        {
                return collect( parent, convert_key( k ), convert_arg( arg ) );
        }

        template < typename Key, typename Arg >
        testing_node_id collect( const Key& k, const Arg& arg )
        {
                return collect( 0, k, arg );
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
        void report_wrong_type_error( const testing_key& );

        template < typename T >
        std::optional< T > extract_arg( const testing_arg_variant& var, const testing_key& key )
        {
                if ( !std::holds_alternative< T >( var ) ) {
                        report_wrong_type_error( key );
                        return {};
                }
                return std::get< T >( var );
        }

        testing_key convert_key( const alternative_of< testing_key > auto& k )
        {
                return testing_key{ k };
        }

        testing_key convert_key( std::string_view k )
        {
                return testing_key{ testing_key_to_buffer( k ) };
        }

        testing_arg_variant convert_arg( const alternative_of< testing_arg_variant > auto& arg )
        {
                return testing_arg_variant{ arg };
        }

        testing_arg_variant convert_arg( std::string_view arg )
        {
                return testing_string_to_buffer( arg );
        }
};
}  // namespace emlabcpp
