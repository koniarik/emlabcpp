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

        void collect( const testing_key& key, const testing_arg_variant& arg );

        void collect( std::string_view key, const testing_arg_variant& arg )
        {
                collect( testing_key_to_buffer( key ), arg );
        }

        void collect( const testing_key& key, std::string_view arg )
        {
                collect( key, testing_string_to_buffer( arg ) );
        }

        void collect( uint32_t key, std::string_view arg )
        {
                collect( testing_key{ key }, testing_string_to_buffer( arg ) );
        }

        void collect( std::string_view key, std::string_view arg )
        {
                collect( testing_key_to_buffer( key ), testing_string_to_buffer( arg ) );
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
};
}  // namespace emlabcpp
