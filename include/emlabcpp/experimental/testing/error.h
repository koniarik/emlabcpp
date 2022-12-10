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
#ifdef EMLABCPP_USE_MAGIC_ENUM
#include "emlabcpp/enum.h"
#endif
#include "emlabcpp/experimental/testing/protocol.h"
#include "emlabcpp/protocol/streams.h"
#include "emlabcpp/visit.h"

#include <variant>

#pragma once

namespace emlabcpp::testing
{

struct reactor_protocol_error
{
        protocol::error_record rec;
};

struct controller_protocol_error
{
        protocol::error_record rec;
};

struct internal_reactor_error
{
        reactor_error_variant val;
};

struct controller_internal_error
{
        messages_enum msg_id;
};

using error_variant = std::variant<
    reactor_protocol_error,
    controller_protocol_error,
    internal_reactor_error,
    controller_internal_error >;

auto& operator<<( ostreamlike auto& os, const reactor_protocol_error& e )
{
        return os << e.rec;
}

auto& operator<<( ostreamlike auto& os, const controller_protocol_error& e )
{
        return os << e.rec;
}

auto& operator<<( ostreamlike auto& os, const internal_reactor_error& e )
{
        match( e.val, [&os, &e]< typename T >( const T& ) {
                os << T::id;
        } );
        return os;
}

auto& operator<<( ostreamlike auto& os, const controller_internal_error& e )
{
#ifdef EMLABCPP_USE_MAGIC_ENUM
        return os << convert_enum( e.msg_id );
#endif
        return os << e.msg_id;
}

auto& operator<<( ostreamlike auto& os, const error_variant& var )
{
        emlabcpp::visit(
            [&os, &var]( const auto& item ) {
                    os << item;
            },
            var );
        return os;
}

}  // namespace emlabcpp::testing
