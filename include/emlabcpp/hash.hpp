/// MIT License
///
/// Copyright (c) 2025-2026 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
///

#pragma once

#include <cstdint>
#include <span>
#include <string_view>

namespace emlabcpp
{

inline constexpr uint32_t fnv1a_offset_basis = 2'166'136'261U;
inline constexpr uint32_t fnv1a_prime        = 16'777'619U;

[[nodiscard]] constexpr uint32_t
fnv1a( std::span< uint8_t const > data, uint32_t acc = fnv1a_offset_basis ) noexcept
{
        for ( uint8_t const b : data ) {
                acc ^= b;
                acc *= fnv1a_prime;
        }
        return acc;
}

[[nodiscard]] constexpr uint32_t
fnv1a( std::string_view data, uint32_t acc = fnv1a_offset_basis ) noexcept
{
        if consteval {
                for ( char const c : data ) {
                        uint8_t const b = static_cast< uint8_t >( c );
                        acc             = fnv1a( std::span< uint8_t const >{ &b, 1 }, acc );
                }
                return acc;
        }
        return fnv1a(
            std::span< uint8_t const >{
                reinterpret_cast< uint8_t const* >( data.data() ), data.size() },
            acc );
}

}  // namespace emlabcpp
