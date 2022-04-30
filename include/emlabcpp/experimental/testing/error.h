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
            [&]( tag< TESTING_ARG_MISSING_E >, testing_key ) {},
            [&]( tag< TESTING_ARG_WRONG_TYPE_E >, testing_key ) {},
            [&]( tag< TESTING_ARG_WRONG_MESSAGE_E >, testing_messages_enum ) {} );
        return os;
}

inline auto& operator<<( ostreamlike auto& os, const testing_controller_message_error& e )
{
        // TODO: multiple cases of this, maybe abstract away?
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
