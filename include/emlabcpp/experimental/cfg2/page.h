#pragma once

#include "../../result.h"

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

struct activ_page_sel
{
        opt< hdr_state > hdr_st = {};
        std::size_t      idx    = 0;

        void on_hdr( std::size_t i, hdr_state cs ) noexcept
        {
                if ( !hdr_st )
                        hdr_st = cs;
                if ( cs == hdr_st )
                        idx = i;
        }

        result on_raw_hdr( std::size_t i, std::span< std::byte, 2 > b ) noexcept
        {
                auto val = byte_to_hdr( b[0] );
                if ( !val || b[0] != ~b[1] )
                        return result::ERROR;
                on_hdr( i, *val );
                return result::SUCCESS;
        }
};

}  // namespace emlabcpp::cfg