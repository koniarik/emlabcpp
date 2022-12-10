#ifdef EMLABCPP_USE_MAGIC_ENUM

#include <magic_enum.hpp>

#endif

#include <type_traits>

#pragma once

namespace emlabcpp
{

/// Returns a string-like name of the enum value `val`
template < typename Enum >
requires( std::is_enum_v< Enum > ) auto convert_enum( Enum val )
{
#ifdef EMLABCPP_USE_MAGIC_ENUM
        return magic_enum::enum_name( val );
#else
        return std::to_string( val );
#endif
}

}  // namespace emlabcpp
