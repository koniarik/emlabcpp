
#include <cstdint>

#pragma once

namespace emlabcpp
{

/// TODO: this needs tests
inline void* align( void* ptr, std::size_t alignment )
{

        const auto intptr = reinterpret_cast< unsigned long long >( ptr );

        return reinterpret_cast< void* >( ( intptr - 1u + alignment ) & -alignment );
}

}  /// namespace emlabcpp
