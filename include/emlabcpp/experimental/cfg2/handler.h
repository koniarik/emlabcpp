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

#include "../../algorithm.h"
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

template < typename T >
using opt = std::optional< T >;

// each cell in memory is 64b wide, looks like this:
// 1b seq | 31b key | 32b val;
// if `1b` is true, val represents number of cells forming the value

static constexpr uint32_t key_mask     = 0x7FFFFFFF;
static constexpr uint32_t sin_bit_mask = 0x80000000;

static_assert( ~key_mask == sin_bit_mask, "key mask and sin bit mask are not correct" );

using cell                              = uint64_t;
static constexpr std::size_t cell_size  = sizeof( cell );
static constexpr std::size_t hcell_size = sizeof( cell ) / 2;

enum class ser_res : uint8_t
{
        SINGLE,
        MULTI,
};

constexpr uint32_t closest_multiple_of( uint32_t x, uint32_t r ) noexcept
{
        return r * ( ( x + r - 1 ) / r );
}

inline opt< ser_res >
ser_cell( uint32_t key, std::span< std::byte > value, std::span< std::byte, cell_size > dest )
{
        if ( value.empty() || key > sin_bit_mask )
                return {};

        static_assert( sizeof( uint32_t ) == hcell_size );
        if ( value.size() <= hcell_size ) {
                uint32_t val = 0x00;
                std::memcpy( &val, value.data(), value.size() );
                uint64_t tmp = key | sin_bit_mask;
                tmp          = ( tmp << 32 ) + val;

                std::memcpy( dest.data(), &tmp, cell_size );
                return ser_res::SINGLE;
        } else {
                uint32_t size =
                    closest_multiple_of( static_cast< uint32_t >( value.size() ), cell_size ) /
                    cell_size;
                uint64_t tmp = key;
                tmp          = ( tmp << 32 ) + size;
                std::memcpy( dest.data(), &tmp, cell_size );
                return ser_res::MULTI;
        }
}

struct deser_res
{
        bool     is_seq;
        uint32_t key;
        uint32_t val;
};

inline opt< deser_res > deser_cell( std::span< std::byte, cell_size > c )
{
        uint64_t tmp = 0x00;
        std::memcpy( &tmp, c.data(), c.size() );
        auto      front = static_cast< uint32_t >( tmp >> 32 );
        deser_res r{
            .is_seq = !static_cast< bool >( front & sin_bit_mask ),
            .key    = static_cast< uint32_t >( front & key_mask ),
            .val    = static_cast< uint32_t >( tmp & 0xFFFF'FFFF ),
        };

        if ( r.is_seq && r.val == 0 )
                return {};
        return r;
}

inline opt< std::span< std::byte > >
store_kval_impl( uint32_t key, std::byte* beg, std::byte* val_end, std::byte* end )
{
        if ( end - val_end < static_cast< int >( cell_size ) )
                return {};
        std::byte* new_end = val_end + cell_size;

        auto tmp = ser_cell(
            key, { beg, val_end }, std::span< std::byte, cell_size >{ val_end, cell_size } );
        if ( !tmp )
                return {};
        switch ( *tmp ) {
        case ser_res::MULTI:
                return std::span{ beg, new_end };
        case ser_res::SINGLE:
                return std::span{ val_end, cell_size };
        }
        return {};
}

template < typename T >
opt< std::span< std::byte > >
store_kval( uint32_t key, T const& val, std::span< std::byte > buffer )
{
        using conv = protocol::converter_for< T, std::endian::little >;  // XXX: make endianness
                                                                         // configurable?
        if ( buffer.size() < conv::max_size )
                return {};
        std::span< std::byte, conv::max_size > front{ buffer.data(), conv::max_size };

        bounded const used = conv::serialize_at( front, val );

        return store_kval_impl(
            key, buffer.data(), buffer.data() + *used, buffer.data() + buffer.size() );
}

template < typename T >
opt< T > get_val( std::span< std::byte > data )
{

        using conv = protocol::converter_for< T, std::endian::little >;  // XXX: make endianness
                                                                         // configurable?
        T res;
        if ( conv::deserialize( data, res ).has_error() )
                return {};
        return res;
}

// iterates over each cell in the page, skipping duplicates
template < typename KeyCache >
struct page_loader
{

        page_loader( std::size_t end_addr, KeyCache& key_cache, std::span< std::byte > buffer )
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

        void on_cell(
            std::span< std::byte, cell_size >                         c,
            std::invocable< uint32_t, std::span< std::byte > > auto&& on_kval )
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
                            if ( skip ) {  // XXX: test
                                    if ( is_seq )
                                            decr( val * sizeof( cell ) );
                                    return cell_st{};
                            }

                            if ( is_seq ) {
                                    if ( val * sizeof( cell ) > buffer_.size() )
                                            return err_st{};
                                    auto x = val * sizeof( cell );
                                    return seq_st{ .key = key, .i = x, .size = x };
                            }
                            std::byte tmp2[sizeof( val )];
                            std::memcpy( tmp2, &val, sizeof( val ) );
                            on_kval( key, std::span{ tmp2 } );
                            return cell_st{};
                    },
                    [&]( seq_st& st ) -> st_var {
                            st.i -= sizeof( cell );
                            std::memcpy( &buffer_[st.i], c.data(), c.size() );
                            if ( st.i == 0 ) {
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
                std::size_t i;
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

        std::size_t            addr_;
        KeyCache               key_cache_;
        std::span< std::byte > buffer_;
};

inline bool is_free_cell( std::span< std::byte, cell_size > cell )
{
        return all_of( cell, [&]( auto x ) {
                return x == std::byte{ 00 };
        } );
}

}  // namespace emlabcpp::cfg
