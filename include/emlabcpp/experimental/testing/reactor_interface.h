#include "emlabcpp/experimental/testing/protocol.h"

#pragma once

namespace emlabcpp
{

class testing_reactor_interface
{
public:
        virtual void                         transmit( std::span< uint8_t > ) = 0;
        virtual static_vector< uint8_t, 64 > read( std::size_t )              = 0;

        static constexpr std::size_t read_limit_ = 10;

        std::optional< testing_controller_reactor_msg > read_message();

        std::optional< testing_controller_reactor_variant > read_variant();

        void reply( const testing_reactor_controller_variant& );

        template < testing_messages_enum ID, typename... Args >
        void reply( const Args&... args )
        {
                reply( testing_reactor_controller_group::make_val< ID >( args... ) );
        }

        void report_failure( testing_error_enum fenum );

        virtual ~testing_reactor_interface() = default;
};
}  // namespace emlabcpp
