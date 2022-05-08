
#include "emlabcpp/experimental/testing/reactor_interface.h"

#pragma once

namespace emlabcpp
{
class testing_reactor_interface_adapter
{
        testing_reactor_interface&                    iface_;
        testing_controller_reactor_packet::sequencer& seq_;

        static constexpr std::size_t read_limit_ = 10;

public:
        testing_reactor_interface_adapter(
            testing_reactor_interface&                    iface,
            testing_controller_reactor_packet::sequencer& seq )
          : iface_( iface )
          , seq_( seq )
        {
        }

        testing_reactor_interface& operator*()
        {
                return iface_;
        }

        testing_reactor_interface* operator->()
        {
                return &iface_;
        }

        std::optional< testing_controller_reactor_variant > read_variant();

        void reply( const testing_reactor_controller_variant& );

        template < testing_messages_enum ID, typename... Args >
        void reply( const Args&... args )
        {
                reply( testing_reactor_controller_group::make_val< ID >( args... ) );
        }

        void report_failure( const testing_reactor_error_variant& );

        template < testing_error_enum ID, typename... Args >
        void report_failure( const Args&... args )
        {
                report_failure( testing_reactor_error_group::make_val< ID >( args... ) );
        }
};
}  // namespace emlabcpp
