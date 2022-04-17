
#include "emlabcpp/experimental/testing/reactor_interface.h"

#pragma once

namespace emlabcpp
{
class testing_reactor_interface_adapter
{
        testing_reactor_interface&    iface_;
        testing_reactor_input_buffer& input_buffer_;

        static constexpr std::size_t read_limit_ = 10;

public:
        testing_reactor_interface_adapter(
            testing_reactor_interface&    iface,
            testing_reactor_input_buffer& input_buffer )
          : iface_( iface )
          , input_buffer_( input_buffer )
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

        void report_failure( testing_error_enum fenum );
};
}  // namespace emlabcpp
