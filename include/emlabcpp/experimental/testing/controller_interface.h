#include "emlabcpp/experimental/testing/error.h"
#include "emlabcpp/experimental/testing/protocol.h"

#pragma once

namespace emlabcpp
{

class testing_controller_interface
{
public:
        virtual void                                          transmit( std::span< uint8_t > )  = 0;
        virtual std::optional< static_vector< uint8_t, 64 > > read( std::size_t )               = 0;
        virtual void                                          on_result( testing_result )       = 0;
        virtual std::optional< testing_arg_variant >          on_arg( uint32_t )                = 0;
        virtual std::optional< testing_arg_variant >          on_arg( std::string_view )        = 0;
        virtual void                                          on_error( testing_error_variant ) = 0;

        static constexpr std::size_t read_limit_ = 10;

        void send( const testing_controller_reactor_variant& );

        template < testing_messages_enum ID, typename... Args >
        void send( const Args&... args )
        {
                send( testing_controller_reactor_group::make_val< ID >( args... ) );
        }

        std::optional< testing_reactor_controller_msg > read_message();

        virtual ~testing_controller_interface() = default;
};

}  // namespace emlabcpp
