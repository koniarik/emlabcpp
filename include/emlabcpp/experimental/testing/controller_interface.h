#include "emlabcpp/experimental/testing/error.h"
#include "emlabcpp/experimental/testing/protocol.h"

#pragma once

namespace emlabcpp
{

class testing_controller_interface
{
public:
        virtual void                                          transmit( std::span< uint8_t > )  = 0;
        virtual std::optional< static_vector< uint8_t, 64 > > receive( std::size_t )            = 0;
        virtual void                                          on_result( testing_result )       = 0;
        virtual std::optional< testing_arg_variant >          on_arg( uint32_t )                = 0;
        virtual std::optional< testing_arg_variant >          on_arg( std::string_view )        = 0;
        virtual void                                          on_error( testing_error_variant ) = 0;

        virtual ~testing_controller_interface() = default;
};

}  // namespace emlabcpp
