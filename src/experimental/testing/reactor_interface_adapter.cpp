#include "experimental/testing/reactor_interface_adapter.h"

#include "emlabcpp/protocol/packet_handler.h"

using namespace emlabcpp;

std::optional< testing_controller_reactor_variant >
testing_reactor_interface_adapter::read_variant()
{
        using sequencer = std::decay_t< decltype( seq_ ) >;
        using handler   = protocol_packet_handler< testing_controller_reactor_packet >;

        std::size_t                                     to_read = sequencer::fixed_size;
        std::optional< testing_controller_reactor_msg > opt_msg;
        while ( !opt_msg ) {
                std::optional opt_data = iface_.receive( to_read );
                if ( !opt_data ) {
                        return {};
                }

                seq_.load_data( view{ *opt_data } )
                    .match(
                        [&]( std::size_t next_read ) {
                                to_read = next_read;
                        },
                        [&]( auto msg ) {
                                opt_msg = msg;
                        } );
        }

        if ( !opt_msg ) {
                return {};
        }

        std::optional< testing_controller_reactor_variant > res;
        handler::extract( *opt_msg )
            .match(
                [&]( auto var ) {
                        res = var;
                },
                [&]( protocol_error_record rec ) {
                        reply< TESTING_PROTOCOL_ERROR >( rec );
                } );
        return res;
}
void testing_reactor_interface_adapter::reply( const testing_reactor_controller_variant& var )
{
        using handler = protocol_packet_handler< testing_reactor_controller_packet >;
        auto msg      = handler::serialize( var );
        iface_.transmit( msg );
}

void testing_reactor_interface_adapter::report_failure( testing_error_enum fenum )
{
        reply< TESTING_INTERNAL_ERROR >( fenum );
}
