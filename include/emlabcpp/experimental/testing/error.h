///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
/// and associated documentation files (the "Software"), to deal in the Software without
/// restriction, including without limitation the rights to use, copy, modify, merge, publish,
/// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or
/// substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
/// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
/// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#pragma once

#ifdef EMLABCPP_USE_MAGIC_ENUM
#include "emlabcpp/enum.h"
#endif
#include "emlabcpp/experimental/testing/protocol.h"
#include "emlabcpp/protocol/streams.h"
#include "emlabcpp/visit.h"

#include <variant>

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
        msgid msg_id;
};

using error_variant = std::variant<
    reactor_protocol_error,
    controller_protocol_error,
    internal_reactor_error,
    controller_internal_error >;

#ifdef EMLABCPP_USE_OSTREAM
inline std::ostream& operator<<( std::ostream& os, const reactor_protocol_error& e )
{
        return os << e.rec;
}

inline std::ostream& operator<<( std::ostream& os, const controller_protocol_error& e )
{
        return os << e.rec;
}

inline std::ostream& operator<<( std::ostream& os, const internal_reactor_error& e )
{
        match( e.val, [&os]< typename T >( const T& ) {
#ifdef EMLABCPP_USE_MAGIC_ENUM
                os << convert_enum( T::id );
#else
                os << static_cast<std::underlying_type_t<T>>(T::id);
#endif
        } );
        return os;
}

inline std::ostream& operator<<( std::ostream& os, const controller_internal_error& e )
{
#ifdef EMLABCPP_USE_MAGIC_ENUM
        return os << convert_enum( e.msg_id );
#else
        return os << e.msg_id;
#endif
}

inline std::ostream& operator<<( std::ostream& os, const error_variant& var )
{
        emlabcpp::visit(
            [&os]( const auto& item ) {
                    os << item;
            },
            var );
        return os;
}
#endif

}  // namespace emlabcpp::testing
