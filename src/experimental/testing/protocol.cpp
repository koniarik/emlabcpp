#include "emlabcpp/experimental/testing/protocol.h"

namespace emlabcpp::protocol
{

template class endpoint< testing::controller_reactor_packet, testing::reactor_controller_packet >;
template class endpoint< testing::reactor_controller_packet, testing::controller_reactor_packet >;

}  // namespace emlabcpp::protocol
