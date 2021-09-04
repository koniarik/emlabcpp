#ifdef EMLABCPP_USE_STREAMS

#include "emlabcpp/protocol/base.h"

#ifdef EMLABCPP_USE_MAGIC_ENUM
#include "magic_enum.hpp"
#endif

#pragma once

namespace emlabcpp
{

inline std::ostream& operator<<( std::ostream& os, const protocol_error_record& rec )
{
        return os << em::view{ rec.os } << "::" << em::view{ rec.err } << "::" << rec.byte_index;
}

template < auto ID >
inline std::ostream& operator<<( std::ostream& os, protocol_cmd_tag< ID > )
{
#ifdef EMLABCPP_USE_MAGIC_ENUM
        return os << magic_enum::enum_name( ID );
#else
        return os << ID;
#endif
}

}  // namespace emlabcpp

#endif
