#include "emlabcpp/experimental/testing/protocol.h"

#include "emlabcpp/protocol/packet_handler.h"

namespace emlabcpp::testing
{
using controller_reactor_handler = protocol::packet_handler< controller_reactor_packet >;
using reactor_controller_handler = protocol::packet_handler< reactor_controller_packet >;

reactor_controller_msg reactor_controller_serialize( const reactor_controller_variant& var )
{
        return reactor_controller_handler::serialize( var );
}
either< reactor_controller_variant, protocol::error_record >
reactor_controller_extract( const reactor_controller_msg& msg )
{
        return reactor_controller_handler::extract( msg );
}

controller_reactor_msg controller_reactor_serialize( const controller_reactor_variant& var )
{
        return controller_reactor_handler::serialize( var );
}
either< controller_reactor_variant, protocol::error_record >
controller_reactor_extract( const controller_reactor_msg& msg )
{
        return controller_reactor_handler::extract( msg );
}

}  // namespace emlabcpp::testing
