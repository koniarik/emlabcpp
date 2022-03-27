#include "emlabcpp/experimental/testing/controller_interface.h"

#include "emlabcpp/protocol/packet_handler.h"

using namespace emlabcpp;

void testing_controller_interface::send( const testing_controller_reactor_variant& var )
{
        using handler = protocol_packet_handler< testing_controller_reactor_packet >;
        auto msg      = handler::serialize( var );
        transmit( msg );
}

std::optional< testing_reactor_controller_msg > testing_controller_interface::read_message()
{
        using sequencer = testing_reactor_controller_packet::sequencer;
        return protocol_simple_load< sequencer >( read_limit_, [&]( std::size_t c ) {
                return read( c );
        } );
}
