// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//
//  Copyright © 2022 Jan Veverak Koniarik
//  This file is part of project: emlabcpp
//
#include "emlabcpp/experimental/testing/error.h"
#include "emlabcpp/experimental/testing/protocol.h"

#pragma once

namespace emlabcpp
{

class testing_controller_interface
{
public:
        virtual void                                          transmit( std::span< uint8_t > ) = 0;
        virtual std::optional< static_vector< uint8_t, 64 > > receive( std::size_t )           = 0;
        virtual void                                 on_result( const testing_result& )        = 0;
        virtual std::optional< testing_arg_variant > on_arg( uint32_t )                        = 0;
        virtual std::optional< testing_arg_variant > on_arg( std::string_view )                = 0;
        virtual void                                 on_error( testing_error_variant )         = 0;

        virtual ~testing_controller_interface() = default;
};

}  // namespace emlabcpp
