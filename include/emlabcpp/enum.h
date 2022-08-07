#ifdef EMLABCPP_USE_MAGIC_ENUM

#include <magic_enum.hpp>

#endif

#pragma once

namespace emlabcpp
{

template < typename Enum >
auto convert_enum( Enum val )
{
#ifdef EMLABCPP_USE_MAGIC_ENUM
        return magic_enum::enum_name( val );
#else
        return std::to_string( val );
#endif
}

}  // namespace emlabcpp
