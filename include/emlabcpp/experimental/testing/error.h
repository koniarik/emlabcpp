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
#include "emlabcpp/experimental/testing/protocol.h"
#include "emlabcpp/protocol/streams.h"
#include "emlabcpp/visit.h"

#include <variant>

#ifdef EMLABCPP_USE_MAGIC_ENUM

#include <magic_enum.hpp>

#endif

#pragma once

namespace emlabcpp
{

struct testing_reactor_protocol_error
{
        protocol_error_record rec;
};

struct testing_controller_protocol_error
{
        protocol_error_record rec;
};

struct testing_internal_reactor_error
{
        testing_reactor_error_variant val;
};

struct testing_controller_message_error
{
        testing_messages_enum msg_id;
};

using testing_error_variant = std::variant<
    testing_reactor_protocol_error,
    testing_controller_protocol_error,
    testing_internal_reactor_error,
    testing_controller_message_error >;

inline auto& operator<<( ostreamlike auto& os, const testing_reactor_protocol_error& e )
{
        return os << e.rec;
}

inline auto& operator<<( ostreamlike auto& os, const testing_controller_protocol_error& e )
{
        return os << e.rec;
}

inline auto& operator<<( ostreamlike auto& os, const testing_internal_reactor_error& e )
{
        apply_on_match(
            e.val,
            [&]< auto ID >( tag< ID > ) {
#ifdef EMLABCPP_USE_MAGIC_ENUM
                    os << magic_enum::enum_name( ID );
#else
                    os << std::to_string( ID );
#endif
            },
            [&]( tag< TESTING_NO_RESPONSE_E >, testing_messages_enum ) {},
            [&]( tag< TESTING_ARG_MISSING_E >, const testing_key& ) {},
            [&]( tag< TESTING_ARG_WRONG_TYPE_E >, const testing_key& ) {},
            [&]( tag< TESTING_ARG_WRONG_MESSAGE_E >, testing_messages_enum ) {} );
        return os;
}

inline auto& operator<<( ostreamlike auto& os, const testing_controller_message_error& e )
{
        /// TODO: multiple cases of this, maybe abstract away?
#ifdef EMLABCPP_USE_MAGIC_ENUM
        return os << magic_enum::enum_name( e.msg_id );
#else
        return os << std::to_string( e.msg_id );
#endif
}

inline auto& operator<<( ostreamlike auto& os, const testing_error_variant& var )
{
        emlabcpp::visit(
            [&]( const auto& item ) {
                    os << item;
            },
            var );
        return os;
}

}  // namespace emlabcpp
