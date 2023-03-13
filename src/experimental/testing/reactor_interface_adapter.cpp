#include "emlabcpp/experimental/testing/reactor_interface_adapter.h"

namespace emlabcpp::testing
{

void reactor_interface_adapter::reply( const reactor_controller_variant& var )
{
        using h        = protocol::handler< reactor_controller_group >;
        const auto msg = h::serialize( var );
        transmit_( channel_, msg );
}

void reactor_interface_adapter::report_failure( const reactor_error_variant& evar )
{
        reply( reactor_internal_error_report{ evar } );
}
}  // namespace emlabcpp::testing
