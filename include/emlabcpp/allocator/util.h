
#include <cstdint>
#include <cstddef>

#pragma once

namespace emlabcpp
{

/// TODO: this needs tests
inline void* align( void* ptr, std::size_t alignment )
{
        // note: based on tips from sarah@#include discord
        const auto iptr       = reinterpret_cast< std::uintptr_t >( ptr );
        const auto low_bit_mask = alignment - 1;
        const auto aligned      = ( iptr + low_bit_mask ) & ~low_bit_mask;

        return static_cast< std::byte* >( ptr ) + ( aligned - iptr );
}

}  // namespace emlabcpp
