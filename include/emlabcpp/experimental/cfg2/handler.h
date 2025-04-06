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

#include "../../assert.h"
#include "../../match.h"
#include "../../protocol/converter.h"
#include "../../result.h"

#include <cstdint>
#include <limits>
#include <optional>
#include <span>

namespace emlabcpp::cfg
{

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

inline std::optional< hdr_state > byte_to_hdr( std::byte b ) noexcept
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

// find last matching initial state
struct page_selector
{
        page_selector( hdr_state cs ) noexcept
          : cs_( cs )
        {
        }

        void on_page( std::size_t i, hdr_state cs ) noexcept
        {
                if ( cs != cs_ )
                        return;
                i_ = i;
        }

        std::pair< std::size_t, hdr_state > finish() && noexcept
        {
                return { i_, cs_ };
        }

private:
        std::size_t i_ = 0;
        hdr_state   cs_;
};

struct next_page_locator
{

        bool check_page( std::size_t i, std::byte data[2] ) noexcept
        {
                if ( empty_i_ )
                        return true;
                auto val = byte_to_hdr( data[0] );
                // XXX: the XOR is NOT documented anywhere
                if ( data[0] == std::byte{ 0x00 } || data[0] != ~data[1] || !val ) {
                        empty_i_ = i;
                        return true;
                }

                if ( !psel_ )
                        psel_ = page_selector{ *val };
                else
                        psel_->on_page( i, *val );

                return false;
        }

        std::pair< std::size_t, hdr_state > finish() && noexcept
        {
                if ( psel_ ) {
                        auto [i, st] = std::move( *psel_ ).finish();
                        return { i, next( st ) };
                }
                return { 0, hdr_state::A };
        }

private:
        std::optional< page_selector > psel_;
        std::optional< std::size_t >   empty_i_;
        hdr_state                      state_ = hdr_state::A;
};

// each cell in memory is 64b wide, looks like this:
// 1b seq | 31b key | 32b val;
// if `1b` is true, val represents number of cells forming the value

static constexpr uint32_t key_mask = 0x7FFFFFFF;

static constexpr uint32_t sin_bit_mask = 0x80000000;

static_assert( ~key_mask == sin_bit_mask, "key mask and sin bit mask are not correct" );

using cell = uint64_t;

static constexpr std::size_t cell_size = sizeof( cell );

struct ser_res
{
        uint64_t              cell;
        std::span< uint64_t > extra;
};

inline std::optional< ser_res > ser_cell( uint32_t key, std::span< uint64_t > value )
{
        static constexpr std::size_t u32max = std::numeric_limits< uint32_t >::max();
        if ( value.empty() || key > sin_bit_mask || value.size() > u32max )
                return {};

        std::size_t cell_n = value.size() > 1 ? value.size() + 1 :  //
                                 value[0] > u32max ? 2 :
                                                     1;
        auto        u64k   = static_cast< uint64_t >( key );
        uint64_t    cell;
        if ( cell_n == 1 )
                cell = ( ( u64k | sin_bit_mask ) << 32 ) + value[0];
        else
                cell = ( u64k << 32 ) + value.size();

        return ser_res{
            .cell  = cell,
            .extra = cell_n == 1 ? std::span< uint64_t >{} : value,
        };
}

struct load_res
{
        bool     is_seq;
        uint32_t key;
        uint32_t val;
};

inline std::optional< load_res > deser_cell( uint64_t c )
{
        auto     front = static_cast< uint32_t >( c >> 32 );
        load_res r{
            .is_seq = !static_cast< bool >( front & sin_bit_mask ),
            .key    = static_cast< uint32_t >( front & key_mask ),
            .val    = static_cast< uint32_t >( c & 0xFFFFFFFF ),
        };

        if ( r.is_seq && r.val == 0 )
                return {};
        return r;
}

inline result store_kval_impl(
    uint32_t               key,
    std::span< uint8_t >   front,
    std::span< uint64_t >  buffer,
    std::span< uint64_t >& used_area )
{
        EMLABCPP_ASSERT(
            static_cast< void* >( front.data() ) == static_cast< void* >( buffer.data() ) );

        std::size_t const cell_n = 1 + ( front.size() + cell_size - 1 ) / cell_size;
        if ( cell_n >= buffer.size() )
                return result::ERROR;
        std::copy_backward(
            front.data(), front.data() + front.size(), front.data() + cell_n * cell_size );

        auto r = ser_cell( key, { buffer.data() + 1, cell_n - 1 } );
        if ( !r )
                return result::ERROR;
        buffer[0] = r->cell;
        used_area = std::span{ buffer.data(), cell_n };
        return result::SUCCESS;
}

template < typename T >
result store_kval(
    uint32_t               key,
    T const&               val,
    std::span< uint64_t >  buffer,
    std::span< uint64_t >& used_area )
{
        using conv = protocol::converter_for< T, std::endian::little >;  // XXX: make endianness
                                                                         // configurable?
        if ( buffer.size_bytes() < conv::max_size )
                return result::ERROR;
        std::span< uint8_t, conv::max_size > front{ buffer.data() };

        bounded const used = conv::serialize_at( front, val );
        return store_kval_impl( key, front.subspan( 0, *used ), buffer, used_area );
}

// iterates over each cell in the page, skipping duplicates
template < typename KeyCache >
struct page_loader
{
        using cell = uint64_t;

        page_loader( std::size_t end_addr, KeyCache& key_cache, std::span< uint64_t > buffer )
          : addr_( end_addr )
          , key_cache_( key_cache )
          , buffer_( buffer )
        {
                decr( sizeof( cell ) );
        }

        std::size_t addr_to_read() const noexcept
        {
                return addr_;
        }

        bool errored() const
        {
                return std::holds_alternative< err_st >( state_ );
        }

        void on_cell( cell c, std::invocable< uint32_t, std::span< uint64_t > > auto&& on_kval )
        {
                decr( sizeof( cell ) );
                state_ = match(
                    state_,
                    [&]( err_st& ) -> st_var {
                            return err_st{};
                    },
                    [&]( cell_st& ) -> st_var {
                            auto tmp = deser_cell( c );
                            if ( !tmp )
                                    return cell_st{};
                            auto [is_seq, key, val] = *tmp;
                            bool skip               = key_cache_( key );
                            if ( skip ) {
                                    if ( is_seq )
                                            decr( val * sizeof( cell ) );
                                    return cell_st{};
                            }

                            if ( is_seq ) {
                                    if ( val > buffer_.size() )
                                            return err_st{};
                                    return seq_st{ .key = key, .size = val };
                            }
                            uint64_t u64val = val;
                            on_kval( key, std::span{ &u64val, 1 } );
                            return cell_st{};
                    },
                    [&]( seq_st& st ) -> st_var {
                            buffer_[st.i++] = c;
                            if ( st.i == st.size ) {
                                    on_kval( st.key, { buffer_.data(), st.size } );
                                    return cell_st{};
                            }
                            return st;
                    } );
        }

private:
        struct cell_st
        {
        };

        struct seq_st
        {
                uint32_t    key;
                std::size_t i = 0;
                std::size_t size;
        };

        struct err_st
        {
        };

        using st_var = std::variant< cell_st, seq_st, err_st >;
        st_var state_{ cell_st{} };

        void decr( std::size_t dec )
        {
                if ( addr_ < dec )
                        addr_ = 0;
                else
                        addr_ -= dec;
        }

        std::size_t           addr_;
        KeyCache              key_cache_;
        std::span< uint64_t > buffer_;
};

inline bool is_free_cell( uint64_t cell )
{
        return cell == 0;
}

}  // namespace emlabcpp::cfg
