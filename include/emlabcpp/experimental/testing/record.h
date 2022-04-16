#include "emlabcpp/experimental/testing/base.h"
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

        testing_arg_variant get_arg( std::string_view key )
        {
                return get_arg( testing_key_to_buffer( key ) );
        }

        testing_arg_variant get_arg( uint32_t v )
        {
                return get_arg( testing_key{ v } );
        }

        testing_arg_variant get_arg( testing_key key );

        bool errored()
        {
                return errored_;
        }

        void collect( testing_key key, testing_arg_variant arg );

        void collect( std::string_view key, testing_arg_variant arg )
        {
                collect( testing_key_to_buffer( key ), arg );
        }

        void collect( testing_key key, std::string_view arg )
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
};
}  // namespace emlabcpp
