#include "emlabcpp/experimental/testing/protocol.h"

#include "emlabcpp/protocol/packet_handler.h"

namespace emlabcpp
{
using controller_reactor_handler =
    protocol::protocol_packet_handler< testing_controller_reactor_packet >;
using reactor_controller_handler =
    protocol::protocol_packet_handler< testing_reactor_controller_packet >;

testing_reactor_controller_msg
testing_reactor_controller_serialize( const testing_reactor_controller_variant& var )
{
        return reactor_controller_handler::serialize( var );
}
either< testing_reactor_controller_variant, protocol::protocol_error_record >
testing_reactor_controller_extract( const testing_reactor_controller_msg& msg )
{
        return reactor_controller_handler::extract( msg );
}

testing_controller_reactor_msg
testing_controller_reactor_serialize( const testing_controller_reactor_variant& var )
{
        return controller_reactor_handler::serialize( var );
}
either< testing_controller_reactor_variant, protocol::protocol_error_record >
testing_controller_reactor_extract( const testing_controller_reactor_msg& msg )
{
        return controller_reactor_handler::extract( msg );
}

}  // namespace emlabcpp
