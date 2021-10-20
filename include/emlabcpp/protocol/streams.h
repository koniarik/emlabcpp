#ifdef EMLABCPP_USE_STREAMS

#include "emlabcpp/iterators/numeric.h"
#include "emlabcpp/protocol/base.h"
#include "emlabcpp/protocol/register_map.h"

#include <iomanip>

#ifdef EMLABCPP_USE_MAGIC_ENUM

#include <magic_enum.hpp>

#endif

#pragma once

namespace emlabcpp
{

template < std::size_t N >
inline std::ostream& operator<<( std::ostream& os, const protocol_message< N >& msg )
{
        std::ios_base::fmtflags f( os.flags() );
        os << std::hex << std::setfill( '0' ) << std::setw( 2 );
        std::string l = "";
        for ( uint8_t v : msg ) {
                os << l << int( v );
                l = " ";
        }
        os.flags( f );
        return os;
}

inline std::ostream& operator<<( std::ostream& os, const protocol_mark& m )
{
        for ( char c : m ) {
                os << c;
        }
        return os;
}

inline std::ostream& operator<<( std::ostream& os, const protocol_error_record& rec )
{
        return os << rec.mark << "(" << rec.offset << ")";
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

template < typename... Regs >
inline std::ostream& operator<<( std::ostream& os, const protocol_register_map< Regs... >& m )
{
        using map = protocol_register_map< Regs... >;
        for_each_index< sizeof...( Regs ) >( [&]< std::size_t i >() {
                static constexpr auto key = map::register_key( bounded_constant< i > );
                const auto&           val = m.template get_val< key >();

#ifdef EMLABCPP_USE_MAGIC_ENUM
                os << magic_enum::enum_name( key );
#else
                os << key;
#endif
                os << "\t" << val << "\n";
        } );
        return os;
}

}  // namespace emlabcpp

#endif
