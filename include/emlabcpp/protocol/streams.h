#ifdef EMLABCPP_USE_STREAMS

#include "emlabcpp/protocol/base.h"

#pragma once

namespace emlabcpp
{
inline std::ostream& operator<<( std::ostream& os, const protocol_error_record& rec )
{
        for ( char c : rec.ns ) {
                os << c;
        }
        os << "::";
        for ( char c : rec.err ) {
                os << c;
        }
        return os << " (" << rec.byte_index << ")";
}

inline std::ostream& operator<<( std::ostream& os, const protocol_endianess_enum& val )
{
        switch ( val ) {
                case PROTOCOL_BIG_ENDIAN:
                        return os << "big endian";
                case PROTOCOL_LITTLE_ENDIAN:
                        return os << "little endian";
                case PROTOCOL_PARENT_ENDIAN:
                        return os << "parent's endian";
        }
        return os;
}

}  // namespace emlabcpp

#endif
