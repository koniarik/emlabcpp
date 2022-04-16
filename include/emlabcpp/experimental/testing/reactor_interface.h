#include "emlabcpp/experimental/testing/protocol.h"

#pragma once

namespace emlabcpp
{

class testing_reactor_interface
{
public:
        virtual void transmit( std::span< uint8_t > ) = 0;

        virtual ~testing_reactor_interface() = default;
};
}  // namespace emlabcpp
