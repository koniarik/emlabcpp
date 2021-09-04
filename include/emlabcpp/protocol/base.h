#include "emlabcpp/bounded.h"
#include "emlabcpp/quantity.h"

#include <cstring>
#include <type_traits>

#pragma once

namespace emlabcpp
{

using protocol_mark = std::array< char, 8 >;

struct protocol_error_record
{
        // Static sized string: namespace of the error type, focus on choosing unique namespace for
        // each project using this library, to distinguish the errors between libraries
        protocol_mark ns;
        // Static sized string: name of the error that happend
        protocol_mark err;
        // Index position of the error
        std::size_t byte_index;

        friend constexpr bool
        operator==( const protocol_error_record&, const protocol_error_record& ) = default;
};

inline constexpr protocol_mark make_protocol_mark( const char ( &msg )[9] )
{
        protocol_mark res;
        std::copy_n( msg, res.size(), res.begin() );
        return res;
}

static constexpr auto PROTOCOL_NS = make_protocol_mark( "PROTO   " );

static constexpr auto LOWSIZE_ERR  = make_protocol_mark( "LOWSIZE " );
static constexpr auto BIGSIZE_ERR  = make_protocol_mark( "BIGSIZE " );
static constexpr auto BOUND_ERR    = make_protocol_mark( "BOUND   " );
static constexpr auto UNDEFVAR_ERR = make_protocol_mark( "UNDEFVAR" );

template < bounded_derived size_type, typename T >
struct protocol_result
{
        size_type used;
        T         val;
};

template < typename T >
concept protocol_base_type = std::is_integral_v< T > || std::is_enum_v< T >;

enum class protocol_endianess_enum
{
        BIG,
        LITTLE,
        PARENT
};

template < typename >
inline constexpr protocol_endianess_enum protocol_endianess = protocol_endianess_enum::PARENT;

template < typename CounterType, typename T >
struct protocol_sized_buffer
{
        using counter_type = CounterType;
        using value_type   = T;
};

template < typename Tag, typename T, T Offset >
class protocol_offset : public quantity< protocol_offset< Tag, T, Offset >, T >
{
public:
        static constexpr T offset = Offset;
        using quantity< protocol_offset< Tag, T, Offset >, T >::quantity;
};

#ifdef EMLABCPP_USE_STREAMS
// TODO: maybe put this in different header?
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
                case protocol_endianess_enum::BIG:
                        return os << "big endian";
                case protocol_endianess_enum::LITTLE:
                        return os << "little endian";
                case protocol_endianess_enum::PARENT:
                        return os << "parent's endian";
        }
        return os;
}
#endif

}  // namespace emlabcpp
