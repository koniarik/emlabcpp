#include "emlabcpp/experimental/testing/protocol.h"

#pragma once

namespace emlabcpp
{

class testing_reactor_comm_interface
{
public:
        virtual void                         transmit( std::span< uint8_t > ) = 0;
        virtual static_vector< uint8_t, 64 > read( std::size_t )              = 0;

        static constexpr std::size_t read_limit_ = 10;

        std::optional< testing_controller_reactor_msg > read_message()
        {
                using sequencer = testing_controller_reactor_packet::sequencer;
                return protocol_simple_load< sequencer >( read_limit_, [&]( std::size_t c ) {
                        return read( c );
                } );
        }

        std::optional< testing_controller_reactor_variant > read_variant()
        {
                using handler = protocol_packet_handler< testing_controller_reactor_packet >;
                auto                                                opt_msg = read_message();
                std::optional< testing_controller_reactor_variant > res;
                if ( !opt_msg ) {
                        return res;
                }
                handler::extract( *opt_msg )
                    .match(
                        [&]( auto var ) {
                                res = var;
                        },
                        [&]( protocol_error_record ) {
                                // TODO: add error handling
                        } );
                return res;
        }

        template < testing_messages_enum ID, typename... Args >
        void reply( const Args&... args )
        {
                using handler = protocol_packet_handler< testing_reactor_controller_packet >;
                auto msg      = handler::serialize(
                    testing_reactor_controller_group::make_val< ID >( args... ) );
                transmit( msg );
        }

        void report_failure( testing_error_enum fenum )
        {
                reply< TESTING_INTERNAL_ERROR >( fenum );
        }

        virtual ~testing_reactor_comm_interface() = default;
};
}  // namespace emlabcpp
