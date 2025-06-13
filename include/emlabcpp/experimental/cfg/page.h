/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
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
#pragma once

#include "../../result.h"
#include "./base.h"

#include <cstdint>
#include <optional>

namespace emlabcpp::cfg
{

template < typename T >
using opt = std::optional< T >;

enum class hdr_state : uint8_t
{
        A = 0x40,
        B = A + 0x40,
        C = B + 0x40,
};

inline hdr_state next( hdr_state cs ) noexcept
{
        switch ( cs ) {
        case hdr_state::A:
                return hdr_state::B;
        case hdr_state::B:
                return hdr_state::C;
        case hdr_state::C:
                return hdr_state::A;
        default:
                return cs;
        }
}

inline opt< hdr_state > byte_to_hdr( std::byte b ) noexcept
{
        switch ( b ) {
        case std::byte{ static_cast< uint8_t >( hdr_state::A ) }:
                return hdr_state::A;
        case std::byte{ static_cast< uint8_t >( hdr_state::B ) }:
                return hdr_state::B;
        case std::byte{ static_cast< uint8_t >( hdr_state::C ) }:
                return hdr_state::C;
        default:
                return {};
        }
}

inline opt< hdr_state > hdr_to_hdr_state( std::span< std::byte, cell_size > b ) noexcept
{
        if ( b[0] != ~b[1] )
                return {};
        return byte_to_hdr( b[0] );
}

inline std::array< std::byte, 2 > get_hdr( hdr_state hst ) noexcept
{
        std::array< std::byte, 2 > b;
        b[0] = static_cast< std::byte >( hst );
        b[1] = ~static_cast< std::byte >( hst );
        return b;
}

}  // namespace emlabcpp::cfg
