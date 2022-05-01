
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

template < ostreamlike Stream, std::size_t N >
inline auto& operator<<( Stream& os, const protocol_message< N >& msg )
{
        std::ios_base::fmtflags f( os.flags() );
        os << std::hex;
        char l = '|';
        for ( std::size_t i : range( msg.size() ) ) {
                if ( i % 4 == 0 ) {
                        l = '|';
                }
                os << l << std::setfill( '0' ) << std::setw( 2 ) << int( msg[i] );
                l = ':';
        }
        os.flags( f );
        return os;
}

inline auto& operator<<( ostreamlike auto& os, const protocol_mark& m )
{
        for ( char c : m ) {
                os << c;
        }
        return os;
}

inline auto& operator<<( ostreamlike auto& os, const protocol_error_record& rec )
{
        return os << rec.mark << "(" << rec.offset << ")";
}

inline auto& operator<<( ostreamlike auto& os, const protocol_endianess_enum& val )
{
        switch ( val ) {
                case PROTOCOL_BIG_ENDIAN:
                        return os << "big endian";
                case PROTOCOL_LITTLE_ENDIAN:
                        return os << "little endian";
        }
        return os;
}

template < ostreamlike Stream, protocol_endianess_enum Endianess, typename... Regs >
inline auto& operator<<( Stream& os, const protocol_register_map< Endianess, Regs... >& m )
{
        using map = protocol_register_map< Endianess, Regs... >;

        auto key_to_str = []( auto key ) {
#ifdef EMLABCPP_USE_MAGIC_ENUM
                return magic_enum::enum_name( key );
#else
                return std::to_string( key );
#endif
        };

        std::size_t max_key_size = 0;

        for_each_index< sizeof...( Regs ) >( [&]< std::size_t i >() {
                static constexpr auto key = map::register_key( bounded_constant< i > );
                max_key_size              = key_to_str( key ).size();
        } );

        for_each_index< sizeof...( Regs ) >( [&]< std::size_t i >() {
                static constexpr auto key = map::register_key( bounded_constant< i > );
                const auto&           val = m.template get_val< key >();

                os << std::left << std::setw( static_cast< int >( max_key_size ) )
                   << key_to_str( key ) << "\t" << val << "\n";
        } );
        return os;
}

}  /// namespace emlabcpp
