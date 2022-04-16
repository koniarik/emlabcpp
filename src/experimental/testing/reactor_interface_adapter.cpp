#include "experimental/testing/reactor_interface_adapter.h"

#include "emlabcpp/protocol/packet_handler.h"

using namespace emlabcpp;

std::optional< testing_controller_reactor_msg > testing_reactor_interface_adapter::read_message()
{
        if ( input_buffer_.empty() ) {
                return {};
        }
        return input_buffer_.take_front();
}

std::optional< testing_controller_reactor_variant >
testing_reactor_interface_adapter::read_variant()
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
