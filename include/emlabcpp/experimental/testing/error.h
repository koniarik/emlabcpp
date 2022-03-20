#include "emlabcpp/experimental/testing/protocol.h"
#include "emlabcpp/protocol/streams.h"
#include "emlabcpp/visit.h"

#include <variant>

#ifdef EMLABCPP_USE_STREAMS

#include <ostream>

#ifdef EMLABCPP_USE_MAGIC_ENUM

#include <magic_enum.hpp>

#endif

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
        testing_error_enum val;
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

#ifdef EMLABCPP_USE_STREAMS

inline std::ostream& operator<<( std::ostream& os, const testing_reactor_protocol_error& e )
{
        return os << e.rec;
}
inline std::ostream& operator<<( std::ostream& os, const testing_controller_protocol_error& e )
{
        return os << e.rec;
}
inline std::ostream& operator<<( std::ostream& os, const testing_internal_reactor_error& e )
{
#ifdef EMLABCPP_USE_MAGIC_ENUM
        return os << magic_enum::enum_name( e.val );
#else
        return os << std::to_string( e.val );
#endif
}
inline std::ostream& operator<<( std::ostream& os, const testing_controller_message_error& e )
{
        // TODO: multiple cases of this, maybe abstract away?
#ifdef EMLABCPP_USE_MAGIC_ENUM
        return os << magic_enum::enum_name( e.msg_id );
#else
        return os << std::to_string( e.msg_id );
#endif
}

inline std::ostream& operator<<( std::ostream& os, const testing_error_variant& var )
{
        visit(
            [&]( const auto& item ) {
                    os << item;
            },
            var );
        return os;
}

#endif

}  // namespace emlabcpp
