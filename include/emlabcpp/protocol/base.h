#include "emlabcpp/bounded.h"
#include "emlabcpp/protocol/message.h"
#include "emlabcpp/quantity.h"
#include "emlabcpp/types.h"

#include <bitset>
#include <cstring>
#include <type_traits>
#include <variant>

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
static constexpr auto BADVAL_ERR   = make_protocol_mark( "BADVAL  " );
static constexpr auto GROUP_ERR    = make_protocol_mark( "GRPMATCH" );

template < bounded_derived size_type, typename T >
struct protocol_result
{
        size_type used;
        T         val;
};

template < typename T >
concept protocol_base_type = std::is_integral_v< T > || std::is_enum_v< T >;

enum protocol_endianess_enum
{
        PROTOCOL_BIG_ENDIAN,
        PROTOCOL_LITTLE_ENDIAN,
        PROTOCOL_PARENT_ENDIAN
};
// whe able to move to GCC11: using enum protocol_endianess_enum; and make it enum class

template < typename >
inline constexpr protocol_endianess_enum protocol_endianess = PROTOCOL_PARENT_ENDIAN;

template < typename... Ts >
struct protocol_tuple
{
        using value_type = std::tuple< Ts... >;
};

template < typename... Ts >
struct protocol_group
{
        using options_type = std::variant< Ts... >;
};

template < typename CounterType, typename T >
struct protocol_sized_buffer
{
        using counter_type = CounterType;
        using value_type   = T;
};

template < typename T, T Offset >
struct protocol_offset
{
        static constexpr T offset = Offset;
        using value_type          = T;
};

template < typename T >
struct protocol_item_decl;

template < typename T >
concept protocol_itemizable = std::default_initializable< T > && requires( T val )
{
        protocol_item_decl< T >{};
};

template < protocol_base_type T >
struct protocol_item_decl< T >
{
        using value_type                      = T;
        static constexpr std::size_t max_size = sizeof( T );
};

template < protocol_itemizable T, std::size_t N >
struct protocol_item_decl< std::array< T, N > >
{
        using value_type                      = std::array< T, N >;
        static constexpr std::size_t max_size = protocol_item_decl< T >::max_size * N;
};

template < protocol_itemizable... Ts >
struct protocol_item_decl< std::tuple< Ts... > >
{
        using value_type = std::tuple< typename protocol_item_decl< Ts >::value_type... >;
        static constexpr std::size_t max_size = ( protocol_item_decl< Ts >::max_size + ... );
};

template < protocol_itemizable... Ts >
struct protocol_item_decl< std::variant< Ts... > >
{
        using id_item    = uint8_t;
        using id_decl    = protocol_item_decl< id_item >;
        using value_type = std::variant< typename protocol_item_decl< Ts >::value_type... >;

        static constexpr std::size_t max_size =
            id_decl::max_size +
            std::max< std::size_t >( { protocol_item_decl< Ts >::max_size... } );
};

template < std::size_t N >
struct protocol_item_decl< std::bitset< N > >
{
        using value_type = std::bitset< N >;

        static constexpr std::size_t max_size = ( N + 7 ) / 8;
};

template < std::size_t N >
struct protocol_item_decl< protocol_sizeless_message< N > >
{
        using value_type = protocol_sizeless_message< N >;

        static constexpr std::size_t max_size = N;
};

template < protocol_itemizable T, T Offset >
struct protocol_item_decl< protocol_offset< T, Offset > >
{
        using value_type = T;

        static constexpr std::size_t max_size = protocol_item_decl< T >::max_size;
};

template < quantity_derived T >
struct protocol_item_decl< T >
{
        using value_type = T;

        static constexpr std::size_t max_size =
            protocol_item_decl< typename T::value_type >::max_size;
};

template < protocol_itemizable T, T Min, T Max >
struct protocol_item_decl< bounded< T, Min, Max > >
{
        using value_type = bounded< T, Min, Max >;

        static constexpr std::size_t max_size = protocol_item_decl< T >::max_size;
};

template < typename CounterType, typename T >
struct protocol_item_decl< protocol_sized_buffer< CounterType, T > >
{
        using counter_item = protocol_item_decl< CounterType >;
        using sub_item     = protocol_item_decl< T >;
        using value_type   = sub_item::value_type;

        static constexpr std::size_t max_size = counter_item::max_size + sub_item::max_size;
};

template < auto V >
struct protocol_item_decl< tag< V > >
{
        using sub_item   = protocol_item_decl< decltype( V ) >;
        using value_type = tag< V >;

        static constexpr std::size_t max_size = sub_item::max_size;
};

template < typename... Ts >
struct protocol_item_decl< protocol_group< Ts... > >
{
        using value_type = std::variant< typename protocol_item_decl< Ts >::value_type... >;

        static constexpr std::size_t max_size =
            std::max< std::size_t >( { protocol_item_decl< Ts >::max_size... } );
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
                case PROTOCOL_BIG_ENDIAN:
                        return os << "big endian";
                case PROTOCOL_LITTLE_ENDIAN:
                        return os << "little endian";
                case PROTOCOL_PARENT_ENDIAN:
                        return os << "parent's endian";
        }
        return os;
}
#endif

}  // namespace emlabcpp
