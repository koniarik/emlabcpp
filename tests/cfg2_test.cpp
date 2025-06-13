
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

#include "emlabcpp/convert_view.h"
#include "emlabcpp/experimental/cfg/handler.h"
#include "emlabcpp/experimental/cfg/page.h"

#include <cstdint>
#include <gtest/gtest.h>
#include <limits>
#include <source_location>

namespace emlabcpp::cfg
{

std::byte operator"" _b( unsigned long long int val )
{
        assert( val <= std::numeric_limits< uint8_t >::max() );
        return std::byte{ static_cast< uint8_t >( val ) };
}

hdr_state hdr( char c )
{
        switch ( c ) {
        case 'A':
                return hdr_state::A;
        case 'B':
                return hdr_state::B;
        case 'C':
                return hdr_state::C;
        default:
                throw std::runtime_error( "Invalid cell state" );
        }
}

TEST( cfg, closest_multiply_of )
{
        auto test_f = [&]( uint32_t x, uint32_t r, uint32_t expected ) {
                EXPECT_EQ( closest_multiple_of( x, r ), expected )
                    << "closest_multiple_of( " << x << ", " << r << " ) != " << expected;
        };

        test_f( 0, 1, 0 );
        test_f( 1, 1, 1 );
        test_f( 1, 8, 8 );
        test_f( 7, 8, 8 );
        test_f( 8, 8, 8 );
        test_f( 9, 8, 16 );
}

TEST( cfg, seq )
{
        std::array< std::byte, cell_size * 4 > buffer;

        struct buff_api : read_iface
        {
                std::span< std::byte > buffer;

                buff_api( std::span< std::byte > buf )
                  : buffer( buf )
                {
                }

                result read( std::size_t addr, std::span< std::byte, cell_size > data ) override
                {
                        if ( addr + cell_size > buffer.size() )
                                return result::ERROR;
                        std::memcpy( data.data(), &buffer[addr], cell_size );
                        return result::SUCCESS;
                }
        };

        buff_api iface{ buffer };

        auto set = [&]( std::size_t idx, char c ) {
                if ( c == '0' ) {
                        std::memset( &buffer[idx * cell_size], 0x00, cell_size );
                        return;
                }
                auto tmp = get_hdr( hdr( c ) );
                std::memcpy( &buffer[idx * cell_size], tmp.data(), tmp.size() );
        };

        auto test_f =
            [&]( std::string_view inpt, int current_idx, std::size_t next_idx, hdr_state next_st ) {
                    for ( std::size_t i = 0; i < inpt.size(); ++i )
                            set( i, inpt[i] );

                    auto res = locate_current_page( buffer.size(), cell_size, iface );
                    if ( current_idx >= 0 ) {
                            EXPECT_TRUE( res.has_value() ) << inpt;
                            EXPECT_EQ( res.value(), current_idx * cell_size ) << inpt;
                    } else {
                            EXPECT_FALSE( res.has_value() ) << inpt;
                    }

                    auto res2 = locate_next_page( buffer.size(), cell_size, iface );
                    EXPECT_TRUE( res2.has_value() ) << inpt;
                    EXPECT_EQ( res2->first, next_idx * cell_size ) << inpt;
                    EXPECT_EQ( res2->second, next_st );
            };

        test_f( "0000", -1, 0, hdr_state::A );
        test_f( "A000", 0, 1, hdr_state::A );
        test_f( "AAAA", 3, 0, hdr_state::B );
        test_f( "BAAA", 0, 1, hdr_state::B );
        test_f( "BBAA", 1, 2, hdr_state::B );
        test_f( "BBBA", 2, 3, hdr_state::B );
        test_f( "BBBB", 3, 0, hdr_state::C );
        test_f( "CCBB", 1, 2, hdr_state::C );
        test_f( "CCCC", 3, 0, hdr_state::A );
        test_f( "AACC", 1, 2, hdr_state::A );
}

TEST( cfg, zero_cell )
{
        std::byte tmp[cell_size] = { 0x00_b, 0x00_b, 0x00_b, 0x00_b };
        auto      r = deser_cell( std::span< std::byte, cell_size >{ tmp, cell_size } );
        EXPECT_FALSE( r.has_value() );

        EXPECT_TRUE( is_free_cell( tmp ) );
}

TEST( cfg, cell )
{
        auto test_f = [&]( uint32_t key, std::vector< std::byte > value, cell expected ) {
                std::byte tmp[cell_size] = { 0x00_b };

                ser_cell( key, value, std::span< std::byte, cell_size >{ tmp, cell_size } );
                cell tmp2 = 0x00;
                std::memcpy( &tmp2, tmp, cell_size );
                EXPECT_EQ( tmp2, expected ) << std::hex << tmp2 << " != " << expected;

                std::optional< deser_res > lr =
                    deser_cell( std::span< std::byte, cell_size >{ tmp, cell_size } );
                EXPECT_EQ( lr->key, key );
                if ( value.size() > cell_size / 2 ) {
                        EXPECT_TRUE( lr->is_seq );
                        EXPECT_EQ(
                            lr->val,
                            closest_multiple_of(
                                static_cast< uint32_t >( value.size() ), cell_size ) /
                                cell_size );
                } else {
                        uint32_t val = 0x00;
                        std::memcpy( &val, value.data(), value.size() );
                        EXPECT_FALSE( lr->is_seq );
                        EXPECT_EQ( lr->val, val ) << std::hex << lr->val << " != " << val;
                }
        };

        auto test_fs = [&]( uint32_t key, uint32_t value, cell expected ) {
                std::byte tmp[sizeof value];
                std::memcpy( tmp, &value, sizeof value );
                test_f( key, { std::begin( tmp ), std::end( tmp ) }, expected );
        };

        test_f( 0x0, { 0x0_b }, 0x8000'0000'0000'0000 );
        test_f( 0x1, { 0x1_b, 0x2_b }, 0x8000'0001'0000'0201 );
        test_f( 0x2, { 0x2_b, 0x3_b, 0x4_b }, 0x8000'0002'0004'0302 );
        test_fs( 0x7FFF'FFFF, 0xFFFF'FFFF, 0xFFFF'FFFF'FFFF'FFFF );
        test_fs( 0x0FFF'FFFF, 0x1234'5678, 0x8FFF'FFFF'1234'5678 );
        test_fs( 0x1234'5678, 0x1234'5678, 0x9234'5678'1234'5678 );
        test_f(
            0x1234'5678,
            { 0x12_b, 0x56_b, 0xFF_b, 0xFF_b, 0x12_b, 0x56_b, 0xFF_b, 0xFF_b, 0xFF_b },
            0x1234'5678'0000'0002 );
        test_f( 0x1234'5678, { 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b }, 0x1234'5678'0000'0001 );
}

TEST( cfg, storeload )
{
        auto test_f = [&]< typename T >( uint32_t key, T value, std::size_t size ) {
                std::byte buffer[42];
                auto      area = store_kval( key, value, buffer );
                EXPECT_TRUE( area.has_value() );
                EXPECT_EQ( area->size(), size );

                auto tmp = deser_cell(
                    area->subspan( area->size() - cell_size ).template subspan< 0, cell_size >() );
                EXPECT_TRUE( tmp );
                auto [is_seq, k, val] = *tmp;
                EXPECT_EQ( key, k ) << std::hex << key << " != " << k;
                opt< T > v;
                if ( is_seq )
                        v = get_val< T >( area->subspan( 0, val * cell_size ) );
                else {
                        std::byte tmp2[cell_size / 2];
                        std::memcpy( tmp2, &val, sizeof val );
                        v = get_val< T >( std::span{ tmp2, sizeof tmp2 } );
                }
                EXPECT_TRUE( v.has_value() );
                EXPECT_EQ( *v, value );
        };

        test_f( 0x0FFF'FFFF, uint64_t{ 0xFFFF'FFFF'FFFF'FFFF }, 16 );
        test_f( 0x0FFF'FFFF, 3.14F, 8 );
        test_f( 0x0FFF'FFFF, uint32_t{ 42 }, 8 );
}

struct buffer_builder
{
        auto& add( uint32_t key, std::vector< std::byte > value ) &
        {
                std::byte tmp[cell_size];
                auto      res =
                    ser_cell( key, value, std::span< std::byte, cell_size >{ tmp, cell_size } );
                switch ( *res ) {
                case ser_res::SINGLE:
                        break;
                case ser_res::MULTI:
                        std::vector< std::byte > tmp2(
                            closest_multiple_of(
                                static_cast< uint32_t >( value.size() ), cell_size ),
                            0x00_b );
                        std::memcpy( tmp2.data(), value.data(), value.size() );
                        buffer_.insert( buffer_.end(), tmp2.begin(), tmp2.end() );
                        break;
                }
                buffer_.insert( buffer_.end(), std::begin( tmp ), std::end( tmp ) );
                return *this;
        }

        auto add( uint32_t key, std::vector< std::byte > value ) &&
        {
                this->add( key, std::move( value ) );
                return std::move( *this );
        }

        auto build() &&
        {
                return std::move( buffer_ );
        }

        std::vector< std::byte > buffer_;
};

auto mem_read_f( auto& mem )
{
        return [&]( std::size_t addr, std::span< std::byte, cell_size > buffer ) {
                if ( addr + cell_size > mem.size() )
                        return result::ERROR;
                std::memcpy( buffer.data(), &mem[addr], cell_size );
                return result::SUCCESS;
        };
}

auto mem_write_f( auto& mem )
{
        return [&]( std::size_t addr, std::span< std::byte const > buffer ) {
                if ( addr + buffer.size() > mem.size() )
                        return result::ERROR;
                std::memcpy( &mem[addr], buffer.data(), buffer.size() );
                return result::SUCCESS;
        };
}

opt< std::span< std::byte > > unexpected_serialize_key( uint32_t, std::span< std::byte > )
{
        EXPECT_TRUE( false ) << "unexpected serialize_key";
        return {};
}

TEST( cfg, update )
{
        using kv = std::pair< uint32_t, std::vector< std::byte > >;

        struct noupdate_iface : update_iface
        {
                std::span< std::byte > mem;
                std::byte              buffer[42];

                noupdate_iface( std::vector< std::byte >& m )
                  : mem( m )
                {
                }

                std::span< std::byte > get_buffer() override
                {
                        return { buffer, sizeof( buffer ) };
                }

                result read( std::size_t addr, std::span< std::byte, cell_size > data ) override
                {
                        return mem_read_f( mem )( addr, data );
                }

                result write( std::size_t, std::span< std::byte const > ) override
                {
                        EXPECT_TRUE( false ) << "write not expected";
                        return result::SUCCESS;
                }

                cache_res check_key_cache( uint32_t ) override
                {
                        return cache_res::NOT_SEEN;
                }

                bool value_changed( uint32_t, std::span< std::byte const > ) override
                {
                        return false;
                }

                opt< std::span< std::byte const > >
                serialize_key( uint32_t k, std::span< std::byte > buffer ) override
                {
                        return unexpected_serialize_key( k, buffer );
                }

                result reset_keys() override
                {
                        EXPECT_TRUE( false ) << "reset not expected";
                        return result::SUCCESS;
                }

                opt< uint32_t > take_unseen_key() override
                {
                        return {};
                }

                result clear_page( std::size_t ) override
                {
                        EXPECT_TRUE( false ) << "clear not expected";
                        return result::SUCCESS;
                }
        };

        struct collect_loader : load_iface
        {
                std::span< std::byte >                         mem;
                std::byte                                      buffer[42];
                std::map< uint32_t, std::vector< std::byte > > res;

                collect_loader( std::vector< std::byte >& m )
                  : mem( m )
                {
                }

                std::span< std::byte > get_buffer() override
                {
                        return { buffer, sizeof( buffer ) };
                }

                result read( std::size_t addr, std::span< std::byte, cell_size > data ) override
                {
                        return mem_read_f( mem )( addr, data );
                }

                cache_res check_key_cache( uint32_t ) override
                {
                        return cache_res::NOT_SEEN;
                }

                result on_kval( uint32_t key, std::span< std::byte > data ) override
                {
                        EXPECT_FALSE( res.contains( key ) );
                        res[key] = std::vector< std::byte >( data.begin(), data.end() );
                        return result::SUCCESS;
                }
        };

        auto noupdate_f = [&]( std::vector< kv > const& data, std::source_location sl ) {
                buffer_builder bb;
                for ( auto& [key, value] : data )
                        bb.add( key, value );
                auto mem = std::move( bb ).build();

                noupdate_iface lb{ mem };
                update_result  ures = update_stored_config( 0, mem.size(), lb );
                EXPECT_EQ( ures, update_result::SUCCESS );

                collect_loader cl{ mem };

                auto lres = load_stored_config( 0, mem.size(), cl );
                EXPECT_EQ( lres, result::SUCCESS );
                std::map expected{ data.begin(), data.end() };
                EXPECT_EQ( cl.res, expected );

                if ( HasFatalFailure() ) {
                        std::cout << "Test fail: " << sl.file_name() << ":" << sl.line()
                                  << std::endl;
                }
        };

        struct full_update : update_iface
        {
                std::byte                buffer[42];
                std::span< std::byte >   mem;
                std::span< kv const >    data;
                std::vector< uint32_t >& keys;

                full_update(
                    std::span< std::byte >   m,
                    std::span< kv const >    d,
                    std::vector< uint32_t >& k )
                  : mem( m )
                  , data( d )
                  , keys( k )
                {
                }

                std::span< std::byte > get_buffer() override
                {
                        return { buffer, sizeof( buffer ) };
                }

                result read( std::size_t addr, std::span< std::byte, cell_size > data ) override
                {
                        return mem_read_f( mem )( addr, data );
                }

                result write( std::size_t start_addr, std::span< std::byte const > data ) override
                {
                        return mem_write_f( mem )( start_addr, data );
                }

                cache_res check_key_cache( uint32_t ) override
                {
                        return cache_res::NOT_SEEN;
                }

                bool value_changed( uint32_t, std::span< std::byte const > ) override
                {
                        return true;
                }

                opt< std::span< std::byte const > >
                serialize_key( uint32_t k, std::span< std::byte > buffer ) override
                {
                        auto iter = find_if( data, [&]( auto& kv ) {
                                return kv.first == k;
                        } );
                        if ( iter == data.end() )
                                return {};
                        if ( buffer.size() < iter->second.size() )
                                return {};
                        std::memcpy( buffer.data(), iter->second.data(), iter->second.size() );
                        return std::span< std::byte const >{ iter->second };
                }

                opt< uint32_t > take_unseen_key() override
                {
                        return pop_from_container( keys );
                }

                result reset_keys() override
                {
                        EXPECT_TRUE( false ) << "reset not expected";
                        return result::SUCCESS;
                }

                result clear_page( std::size_t addr ) override
                {
                        EXPECT_TRUE( false ) << "clear not expected at addr: " << addr;
                        return result::SUCCESS;
                }
        };

        auto full_update_f = [&]( std::vector< kv > const& data, std::source_location sl ) {
                std::vector< std::byte > mem( 1024, 0x00_b );
                std::vector< uint32_t >  keys;
                for ( auto& [k, v] : data )
                        keys.push_back( k );

                full_update   lb{ mem, data, keys };
                update_result ures = update_stored_config( 0, mem.size(), lb );
                EXPECT_EQ( ures, update_result::SUCCESS );

                if ( HasFatalFailure() ) {
                        std::cout << "Test fail: " << sl.file_name() << ":" << sl.line()
                                  << std::endl;
                }
        };

        auto test_f = [&]( std::vector< kv >    data,
                           std::source_location sl = std::source_location::current() ) {
                noupdate_f( data, sl );
                full_update_f( data, sl );
        };

        // XXX: the fact that we always have to padd this to multiple is a bit weird
        test_f( {} );
        test_f( { { 0x1, { 0x0_b, 0x0_b, 0x0_b, 0x0_b } } } );
        test_f( { { 0x1, { 0x11_b, 0x12_b, 0x13_b, 0x14_b } } } );
        test_f(
            { { 0x1,
                { 0x19_b,
                  0x18_b,
                  0x17_b,
                  0x16_b,
                  0x15_b,
                  0x14_b,
                  0x13_b,
                  0x12_b,
                  0x11_b,
                  0x10_b,
                  0x00_b,
                  0x00_b,
                  0x00_b,
                  0x00_b,
                  0x00_b,
                  0x00_b } } } );
        test_f(
            { { __LINE__, { 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b } },
              { 0x2, { 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b } },
              { 0x3, { 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b } },
              { 0x4, { 0x01_b, 0x02_b, 0x03_b, 0x04_b } } } );

        test_f(
            { { __LINE__, { 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b } },
              { 0x2, { 0xFF_b, 0xFF_b, 0x00_b, 0x00_b } },
              { 0x3, { 0xFF_b, 0xFF_b, 0x00_b, 0x00_b } },
              { 0x4, { 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b } } } );

        test_f(
            { { __LINE__, { 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b } },
              { 0x2, { 0xFF_b, 0xFF_b, 1_b, 2_b, 3_b, 4_b, 5_b, 0x00_b } },
              { 0x3, { 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b } },
              { 0x4, { 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b, 0x00_b, 0x00_b } } } );

        test_f(
            { { __LINE__,
                { 0xFF_b,
                  0xFF_b,
                  12_b,
                  3_b,
                  43_b,
                  43_b,
                  4_b,
                  3_b,
                  4_b,
                  34_b,
                  43_b,
                  0x00_b,
                  0x00_b,
                  0x00_b,
                  0x00_b,
                  0x00_b } },
              { 0x3, { 0xFF_b, 0xFF_b, 0x00_b, 0x00_b } },
              { 0x4, { 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b, 0x00_b, 0x00_b } } } );
}

struct config

{

        struct kval
        {
                uint32_t                 key;
                std::vector< std::byte > value;

                constexpr friend auto
                operator<=>( kval const& lhs, kval const& rhs ) noexcept = default;
        };

        std::vector< kval > pairs;

        void add_keys( std::size_t n )
        {
                for ( std::size_t i = 0; i < n; ++i ) {
                        std::vector< std::byte > value( i + 1, 00_b );
                        for ( std::size_t j = 0; j < value.size(); ++j ) {
                                value[j] = static_cast< std::byte >(
                                    ( i + j ) % std::numeric_limits< uint8_t >::max() );
                        }
                        std::cout << "Adding key: " << pairs.size()
                                  << " value: " << convert_view< int >( value ) << std::endl;
                        pairs.push_back(
                            { static_cast< uint32_t >( pairs.size() ), std::move( value ) } );
                }
        }

        void drop_key( uint32_t key )
        {
                auto iter = find_if( pairs, [&]( auto& kv ) {
                        return kv.key == key;
                } );
                if ( iter != pairs.end() )
                        pairs.erase( iter );
        }

        std::vector< uint32_t > keys() const
        {
                std::vector< uint32_t > keys;
                keys.reserve( pairs.size() );
                for ( auto& kv : pairs )
                        keys.push_back( kv.key );
                return keys;
        }

        std::span< std::byte > value( uint32_t key )
        {
                auto iter = find_if( pairs, [&]( auto& kv ) {
                        return kv.key == key;
                } );
                if ( iter == pairs.end() )
                        return {};
                return std::span< std::byte >( iter->value );
        }

        void vary( uint32_t key )
        {
                for ( auto& b : value( key ) ) {
                        b = static_cast< std::byte >(
                            ( std::to_integer< uint8_t >( b ) + 1 ) %
                            std::numeric_limits< uint8_t >::max() );
                }
        }

        void vary_all()
        {
                for ( auto& kv : pairs )
                        vary( kv.key );
        }

        constexpr friend auto
        operator<=>( config const& lhs, config const& rhs ) noexcept = default;
};

struct page_info
{
        uint32_t          addr;
        std::span< cell > cells;

        bool valid() const
        {
                // header
                if ( cells[0] == 0x00 )
                        return false;

                auto iter = find_if( cells, []( cell c ) {
                        return c == 0x00;
                } );

                return std::all_of( iter, cells.end(), []( cell c ) {
                        return c == 0x00;
                } );
        }

        void clear()
        {
                std::ranges::fill( cells, 0x00 );
        }

        std::size_t used_cells() const
        {
                auto iter = find_if( cells, []( cell c ) {
                        return c == 0x00;
                } );
                return std::distance( cells.begin(), iter );
        }
};

struct memory
{
        std::size_t              page_size = cell_size * 10;
        std::size_t              mem_size  = page_size * 3;
        std::vector< std::byte > buffer    = std::vector< std::byte >( mem_size, 0x00_b );

        std::size_t page_count() const
        {
                return mem_size / page_size;
        }

        std::size_t pages_used() const
        {
                std::size_t used = 0;
                for ( std::size_t i = 0; i < page_count(); ++i ) {
                        auto addr = i * page_size;
                        if ( std::any_of(
                                 buffer.begin() + addr,
                                 buffer.begin() + addr + page_size,
                                 []( std::byte b ) {
                                         return b != 0x00_b;
                                 } ) )
                                ++used;
                }
                return used;
        }

        std::vector< page_info > pages()
        {
                std::vector< page_info > result;
                for ( std::size_t i = 0; i < page_count(); ++i ) {
                        uint32_t addr = i * page_size;
                        result.emplace_back(
                            page_info{
                                .addr  = addr,
                                .cells = std::span< cell >{
                                    reinterpret_cast< cell* >( buffer.data() + addr ),
                                    page_size / cell_size } } );
                }
                return result;
        }

        void print()
        {
                assert( page_size % cell_size == 0 );
                for ( auto p : pages() ) {
                        std::cout << "Page " << p.addr << ": ";
                        for ( auto& c : p.cells )
                                std::cout << std::hex << c << " ";
                        std::cout << std::dec << std::endl;
                }
        }
};

TEST( cfg, integration )
{
        std::srand( 0 );
        config cfg;
        cfg.add_keys( 3 );
        memory mem;

        std::vector< uint32_t > unseen_keys = cfg.keys();

        struct update_impl : update_iface
        {
                config&                  cfg;
                memory&                  mem;
                std::vector< uint32_t >& keys_seen;
                std::vector< std::byte > buff = std::vector< std::byte >( 42, 0x00_b );

                update_impl( config& cfg, memory& mem, std::vector< uint32_t >& keys_seen )
                  : cfg( cfg )
                  , mem( mem )
                  , keys_seen( keys_seen )
                {
                }

                std::span< std::byte > get_buffer() override
                {
                        return buff;
                }

                result read( std::size_t addr, std::span< std::byte, cell_size > data ) override
                {
                        auto r = mem_read_f( mem.buffer )( addr, data );
                        std::cout << "reading " << addr << ":" << convert_view< int >( data )
                                  << std::endl;
                        return r;
                }

                result write( std::size_t start_addr, std::span< std::byte const > data ) override
                {
                        std::cout << "writing " << start_addr << ":" << convert_view< int >( data )
                                  << std::endl;
                        return mem_write_f( mem.buffer )( start_addr, data );
                }

                cache_res check_key_cache( uint32_t key ) override
                {
                        return key_check_unseen_container( keys_seen, key );
                }

                bool value_changed( uint32_t key, std::span< std::byte const > data ) override
                {
                        bool changed = !is_prefix_of_with_zeros( cfg.value( key ), data );
                        if ( changed )
                                std::cout << "value_changed for key " << key << ": "
                                          << convert_view< int >( cfg.value( key ) ) << " vs "
                                          << convert_view< int >( data ) << " = " << changed
                                          << std::endl;
                        return changed;
                }

                opt< std::span< std::byte const > >
                serialize_key( uint32_t key, std::span< std::byte > buffer ) override
                {
                        EXPECT_NE( cfg.value( key ).size(), 0 ) << "Value size should not be zero";
                        std::cout << "serializing key " << key
                                  << " with value: " << convert_view< int >( cfg.value( key ) )
                                  << std::endl;
                        return store_kval( key, cfg.value( key ), buffer );
                }

                opt< uint32_t > take_unseen_key() override
                {
                        return pop_from_container( keys_seen );
                }

                result reset_keys() override
                {
                        std::cout << "resetting keys" << std::endl;
                        keys_seen = cfg.keys();
                        return result::SUCCESS;
                }

                result clear_page( std::size_t addr ) override
                {
                        std::cout << "clearing page at addr: " << addr << std::endl;
                        auto p = mem.pages()[addr / mem.page_size];
                        p.clear();
                        return result::SUCCESS;
                }
        };

        struct load_impl : load_iface
        {
                config&                  cfg;
                memory&                  mem;
                std::vector< uint32_t >& keys_seen;
                std::vector< std::byte > buff = std::vector< std::byte >( 42, 0x00_b );

                load_impl( config& cfg, memory& mem, std::vector< uint32_t >& keys_seen )
                  : cfg( cfg )
                  , mem( mem )
                  , keys_seen( keys_seen )
                {
                }

                std::span< std::byte > get_buffer() override
                {
                        return buff;
                }

                result read( std::size_t addr, std::span< std::byte, cell_size > data ) override
                {
                        auto r = mem_read_f( mem.buffer )( addr, data );
                        std::cout << "lreading " << addr << ":" << convert_view< int >( data )
                                  << std::endl;
                        return r;
                }

                cache_res check_key_cache( uint32_t key ) override
                {
                        std::cout << "checking key " << key << " in load_impl" << std::endl;
                        return key_check_unseen_container( keys_seen, key );
                }

                result on_kval( uint32_t key, std::span< std::byte > data ) override
                {
                        auto&& val = cfg.value( key );
                        if ( val.size() > data.size() ) {
                                std::cout << "Value size mismatch for key " << key << ": expected "
                                          << val.size() << ", got " << data.size() << std::endl;
                                EXPECT_EQ( val.size(), data.size() );
                                return result::ERROR;
                        }
                        std::span< std::byte > data2{ data.begin(), val.size() };
                        bool equal = std::ranges::equal( cfg.value( key ), data2 );
                        EXPECT_TRUE( equal )
                            << "Value for key " << key << " does not match expected value: "
                            << convert_view< int >( cfg.value( key ) ) << " vs "
                            << convert_view< int >( data );
                        bool rest_is_zero =
                            std::ranges::all_of( data.subspan( val.size() ), []( std::byte b ) {
                                    return b == 0x00_b;
                            } );
                        EXPECT_TRUE( rest_is_zero );
                        return result_e::SUCCESS;
                }
        };

        load_impl   li{ cfg, mem, unseen_keys };
        update_impl ub{ cfg, mem, unseen_keys };
        EXPECT_EQ( mem.pages_used(), 0 );

        auto rotate = [&]( uint32_t pages, uint32_t cells ) {
                unseen_keys = cfg.keys();
                auto res    = update( mem.mem_size, mem.page_size, ub );
                EXPECT_EQ( res, result::SUCCESS );
                EXPECT_EQ( mem.pages_used(), pages );
                auto p = mem.pages()[mem.pages_used() - 1];
                EXPECT_TRUE( p.valid() );
                EXPECT_EQ( p.used_cells(), cells );
                mem.print();
                unseen_keys = cfg.keys();
                res         = load( mem.mem_size, mem.page_size, li );
                EXPECT_TRUE( unseen_keys.empty() );
                EXPECT_EQ( res, result::SUCCESS );
        };

        std::cout << "Initial state:" << std::endl;
        rotate( 1, 4 );

        std::cout << "No change:" << std::endl;
        rotate( 1, 4 );

        std::cout << "Vary single key:" << std::endl;
        cfg.vary( 0 );
        rotate( 1, 5 );

        std::cout << "First variation:" << std::endl;
        cfg.vary_all();
        rotate( 1, 8 );

        std::cout << "Second variation:" << std::endl;
        cfg.vary_all();
        rotate( 2, 4 );

        std::cout << "New key" << std::endl;
        cfg.add_keys( 1 );
        rotate( 2, 5 );

        std::cout << "Third variation:" << std::endl;
        cfg.vary_all();
        rotate( 2, 9 );

        std::cout << "Drop key, vary all" << std::endl;
        cfg.drop_key( 0 );
        cfg.vary_all();
        rotate( 3, 4 );

        std::cout << "Fill pages and overflow" << std::endl;
        cfg.vary_all();
        rotate( 3, 7 );
        cfg.vary_all();
        rotate( 3, 10 );
        cfg.vary_all();
        rotate( 3, 10 );

        // XXX: evidently zero values are an issue, get back to it
}

}  // namespace emlabcpp::cfg
