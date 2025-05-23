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
#include "./page.h"

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
        T res{};
        if ( conv::deserialize( data, res ).has_error() )
                return {};
        return res;
}

enum class cache_res
{
        SEEN,
        NOT_SEEN,
};

inline bool is_free_cell( std::span< std::byte, cell_size > cell )
{
        return all_of( cell, [&]( auto x ) {
                return x == std::byte{ 00 };
        } );
}

struct iface_base
{
        virtual std::span< std::byte > get_buffer()                                     = 0;
        virtual result read( std::size_t addr, std::span< std::byte, cell_size > data ) = 0;

        virtual ~iface_base() = default;
};

inline opt< uint16_t >
locate_current_page( std::size_t mem_size, std::size_t page_size, iface_base& iface )
{
        if ( mem_size % page_size != 0 )
                return {};

        activ_page_sel psel;
        for ( uint32_t i = 0; i < mem_size / page_size; i++ ) {
                std::byte data[cell_size] = {};
                auto      addr            = i * page_size;
                if ( iface.read( addr, data ) != result::SUCCESS )
                        return {};
                if ( psel.on_raw_hdr( i, data ) == result::SUCCESS )
                        continue;
        }
        if ( !psel.hdr_st )
                return {};
        return static_cast< uint16_t >( psel.idx * page_size );
}

inline opt< std::pair< std::size_t, hdr_state > >
locate_next_page( std::size_t mem_size, std::size_t page_size, iface_base& iface )
{
        if ( mem_size % page_size != 0 )
                return {};

        std::size_t    addr;
        activ_page_sel psel;
        for ( uint32_t i = 0; i < mem_size / page_size; i++ ) {
                std::byte data[cell_size] = {};
                addr                      = i * page_size;
                if ( iface.read( addr, data ) != result::SUCCESS )
                        return {};
                if ( psel.on_raw_hdr( i, data ) == result::ERROR )
                        break;
        }
        hdr_state page_st = psel.hdr_st ? *psel.hdr_st : hdr_state::A;
        addr              = psel.hdr_st ? psel.idx * page_size : addr;
        return std::make_pair( addr, page_st );
}

struct update_iface : iface_base
{
        virtual result write( std::size_t start_addr, std::span< std::byte > data ) = 0;

        virtual cache_res check_key_cache( uint32_t key )                            = 0;
        virtual bool      value_changed( uint32_t key, std::span< std::byte > data ) = 0;

        virtual opt< std::size_t > serialize_key( uint32_t key, std::span< std::byte > buffer ) = 0;

        virtual result          reset_keys()      = 0;
        virtual opt< uint32_t > take_unseen_key() = 0;
};

template <
    typename Read,
    typename Write,
    typename KeyCheck,
    typename ValueChanged,
    typename SerializeKey,
    typename TakeUnseenKey,
    typename ResetKeys >
struct update_cbs
{
        std::span< std::byte > buffer;
        Read                   read_f;
        Write                  write_f;
        KeyCheck               check_key_cache_f;
        ValueChanged           value_changed_f;
        SerializeKey           serialize_key_f;
        TakeUnseenKey          take_unseen_key_f;
        ResetKeys              reset_keys_f;
};

template < typename T >
struct update_cbs_bind : update_iface
{
        T& lu;

        update_cbs_bind( T& lu )
          : lu( lu )
        {
        }

        std::span< std::byte > get_buffer() override
        {
                return lu.buffer;
        }

        result read( std::size_t addr, std::span< std::byte, cell_size > data ) override
        {
                return lu.read_f( addr, data );
        }

        result write( std::size_t start_addr, std::span< std::byte > data ) override
        {
                return lu.write_f( start_addr, data );
        }

        cache_res check_key_cache( uint32_t key ) override
        {
                return lu.check_key_cache_f( key );
        }

        bool value_changed( uint32_t key, std::span< std::byte > data ) override
        {
                return lu.value_changed_f( key, data );
        }

        opt< std::size_t > serialize_key( uint32_t key, std::span< std::byte > buffer ) override
        {
                return lu.serialize_key_f( key, buffer );
        }

        opt< uint32_t > take_unseen_key() override
        {
                return lu.take_unseen_key_f();
        }

        result reset_keys() override
        {
                return lu.reset_keys_f();
        }
};

enum class update_result : uint8_t
{
        SUCCESS = 0x00,
        ERROR   = 0x01,
        FULL    = 0x02,
};

std::span< std::byte > manifest_value(
    bool  is_seq,
    auto& cell_val,
    auto& decr_addr,
    auto& addr,
    auto& iface,
    auto& buffer )
{
        if ( is_seq ) {
                for ( std::size_t i = 0; i < cell_val; ++i ) {
                        if ( !decr_addr( addr ) )
                                return {};
                        auto buffer_offset = ( cell_val - i - 1 ) * cell_size;
                        if ( buffer_offset + cell_size > buffer.size() )
                                return {};
                        if ( iface.read(
                                 addr,
                                 std::span< std::byte, cell_size >{
                                     buffer.data() + buffer_offset, cell_size } ) !=
                             result::SUCCESS )
                                return {};
                }
                return buffer.subspan( 0, cell_val * cell_size );
        } else {
                std::memcpy( buffer.data(), &cell_val, sizeof( cell_val ) );
                return buffer.subspan( 0, sizeof( cell_val ) );
        }
}

update_result store_key( std::size_t& addr, uint32_t key, update_iface& iface )
{
        std::span< std::byte > buffer   = iface.get_buffer();
        auto                   key_used = iface.serialize_key( key, buffer );
        if ( !key_used )
                return update_result::ERROR;

        if ( *key_used > buffer.size() )
                return update_result::FULL;

        if ( iface.write( addr, buffer.subspan( 0, *key_used ) ) == result::SUCCESS ) {
                addr += *key_used;
                return update_result::SUCCESS;
        } else
                return update_result::ERROR;
}

inline update_result dump_unseen_keys( std::size_t addr, update_iface& iface )
{
        while ( auto k = iface.take_unseen_key() ) {
                auto res = store_key( addr, *k, iface );
                if ( res != update_result::SUCCESS )
                        return res;
        }
        return update_result::SUCCESS;
}

inline update_result
update_stored_config( std::size_t start_addr, std::size_t end_addr, update_iface& iface )
{
        std::byte   tmp[cell_size];
        std::size_t addr = end_addr;

        auto decr_addr = [&]( auto& addr ) {
                if ( addr - cell_size > addr )
                        return false;
                addr -= cell_size;
                if ( addr < start_addr )
                        return false;
                return true;
        };

        std::size_t last_free = end_addr;
        for ( ;; ) {
                if ( !decr_addr( addr ) )
                        break;
                if ( iface.read( addr, std::span< std::byte, cell_size >{ tmp } ) !=
                     result::SUCCESS )
                        return update_result::ERROR;
                if ( is_free_cell( tmp ) ) {
                        last_free = addr;
                        continue;
                }
                addr += cell_size;
                break;
        }

        std::span< std::byte > buffer = iface.get_buffer();

        for ( ;; ) {
                if ( !decr_addr( addr ) )
                        break;
                if ( iface.read( addr, std::span< std::byte, cell_size >{ tmp } ) !=
                     result::SUCCESS )
                        return update_result::ERROR;
                auto c = deser_cell( tmp );
                if ( !c )
                        return update_result::ERROR;
                auto [is_seq, key, val] = *c;
                cache_res cr            = iface.check_key_cache( key );
                if ( cr == cache_res::SEEN ) {
                        if ( is_seq )
                                addr -= val * cell_size;
                        continue;
                }

                std::span< std::byte > val_sp =
                    manifest_value( is_seq, val, decr_addr, addr, iface, buffer );
                bool changed = iface.value_changed( key, val_sp );
                if ( !changed )
                        continue;

                if ( auto res = store_key( addr, key, iface ); res != update_result::SUCCESS )
                        return res;
        }

        return dump_unseen_keys( last_free, iface );
}

inline result update( std::size_t mem_size, std::size_t page_size, update_iface& iface )
{
        auto addr = locate_current_page( mem_size, page_size, iface );
        if ( !addr )
                return result::SUCCESS;
        if ( *addr > mem_size )
                return result::ERROR;
        auto r = update_stored_config( *addr + cell_size, *addr + page_size, iface );
        if ( r != update_result::FULL )
                return r == update_result::SUCCESS ? result::SUCCESS : result::ERROR;
        auto info = locate_next_page( mem_size, page_size, iface );
        if ( !info )
                return result::ERROR;
        {
                auto [addr, page_st] = *info;
                if ( addr > mem_size )
                        return result::ERROR;
                if ( iface.reset_keys() != result::SUCCESS )
                        return result::ERROR;

                auto hdr = get_hdr( page_st );
                if ( iface.write( addr, std::span< std::byte >{ hdr } ) != result::SUCCESS )
                        return result::ERROR;
                addr += cell_size;

                if ( iface.reset_keys() != result::SUCCESS )
                        return result::ERROR;

                return dump_unseen_keys( addr, iface ) == update_result::SUCCESS ? result::SUCCESS :
                                                                                   result::ERROR;
        }
}

struct load_iface : iface_base
{

        virtual cache_res check_key_cache( uint32_t key ) = 0;

        virtual result on_kval( uint32_t key, std::span< std::byte > ) = 0;
};

template < typename Read, typename KeyCheck, typename OnKval >
struct load_cbs
{
        std::span< std::byte > buffer;
        Read                   read_f;
        KeyCheck               check_key_cache_f;
        OnKval                 on_kval_f;
};

template < typename T >
struct load_cbs_bind : load_iface
{
        T& lu;

        load_cbs_bind( T& lu )
          : lu( lu )
        {
        }

        std::span< std::byte > get_buffer() override
        {
                return lu.buffer;
        }

        result read( std::size_t addr, std::span< std::byte, cell_size > data ) override
        {
                return lu.read_f( addr, data );
        }

        cache_res check_key_cache( uint32_t key ) override
        {
                return lu.check_key_cache_f( key );
        }

        result on_kval( uint32_t key, std::span< std::byte > data ) override
        {
                return lu.on_kval_f( key, data );
        }
};

inline result load_stored_config( std::size_t start_addr, std::size_t end_addr, load_iface& iface )
{
        std::size_t addr      = end_addr;
        auto        decr_addr = [&]( auto& addr ) {
                if ( addr - cell_size > addr )
                        return false;
                addr -= cell_size;
                if ( addr < start_addr )
                        return false;
                return true;
        };

        std::byte tmp[cell_size];
        auto      buffer = iface.get_buffer();
        for ( ;; ) {
                if ( !decr_addr( addr ) )
                        break;
                if ( iface.read( addr, std::span< std::byte, cell_size >{ tmp } ) !=
                     result::SUCCESS )
                        return result::ERROR;
                // XXX: copy-pasta from other function
                auto c = deser_cell( tmp );
                if ( !c )
                        continue;
                auto [is_seq, key, val] = *c;
                cache_res cr            = iface.check_key_cache( key );
                if ( cr == cache_res::SEEN ) {
                        if ( is_seq )
                                addr -= val * cell_size;
                        continue;
                }
                std::span< std::byte > val_sp =
                    manifest_value( is_seq, val, decr_addr, addr, iface, buffer );

                if ( iface.on_kval( key, val_sp ) != result::SUCCESS )
                        return result::ERROR;
        }

        return result::SUCCESS;
}

inline result load( std::size_t mem_size, std::size_t page_size, load_iface& iface )
{
        auto addr = locate_current_page( mem_size, page_size, iface );
        if ( !addr )
                return result::SUCCESS;
        if ( *addr > mem_size )
                return result::ERROR;
        return load_stored_config( *addr + cell_size, *addr + page_size, iface );
}

// Util functions usable as callbacks

opt< std::size_t > pop_from_container( auto& cont )
{
        if ( cont.empty() )
                return {};
        auto k = cont.back();
        cont.pop_back();
        return k;
}

cache_res key_check_unseen_container( auto& cont, uint32_t key )
{
        auto iter = find( cont, key );
        if ( iter == cont.end() )
                return cache_res::SEEN;
        std::swap( *iter, cont.back() );
        cont.pop_back();
        return cache_res::NOT_SEEN;
}

template < typename M >
opt< std::size_t > serialize_reg_map_key( M& reg_map, uint32_t key, std::span< std::byte > buffer )
{
        opt< std::size_t > res;
        reg_map.with_register(
            static_cast< typename M::key_type >( key ), [&]< typename Reg >( Reg& reg ) {
                    auto ran = store_kval( Reg::key, reg.value, buffer );
                    if ( ran )
                            res = ran->size();
            } );
        return res;
}

template < typename M >
result set_reg_map_key( M& reg_map, uint32_t key, std::span< std::byte > buffer )
{
        result res = result::ERROR;
        reg_map.with_register(
            static_cast< typename M::key_type >( key ), [&]< typename Reg >( Reg& reg ) {
                    auto val = get_val< typename Reg::value_type >( buffer );
                    if ( val ) {
                            reg.value = *val;
                            res       = result::SUCCESS;
                    }
            } );
        return res;
}

template < typename M >
bool reg_map_value_changed( M& reg_map, uint32_t key, std::span< std::byte > data )
{
        bool res = false;
        reg_map.with_register(
            static_cast< typename M::key_type >( key ), [&]< typename Reg >( Reg& reg ) {
                    auto val = get_val< typename Reg::value_type >( data );
                    if ( val )
                            res = reg.value != *val;
            } );
        return res;
}

// XXX: test utils
// XXX: test case with reg map
// XXX: test scenarios:
//  - new key in keyval map
//  - key dropped from kval map
//  - only part of keys updated - partial write
//  - write triggers full
}  // namespace emlabcpp::cfg
