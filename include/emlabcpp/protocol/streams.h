#ifdef EMLABCPP_USE_STREAMS

#include "emlabcpp/protocol/base.h"

#pragma once

namespace emlabcpp
{

template < std::size_t N >
inline std::ostream& operator<<( std::ostream& os, const protocol_message< N >& msg )
{
        std::ios_base::fmtflags f( os.flags() );
        os << std::hex;
        for ( uint8_t v : msg ) {
                os << int( v );
        }
        os.flags( f );
        return os;
}

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
        }
        return os;
}

}  // namespace emlabcpp

#endif
