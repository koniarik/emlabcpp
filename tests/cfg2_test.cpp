
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
#include "emlabcpp/experimental/cfg2/handler.h"

#include <gtest/gtest.h>

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
        auto test_f = [&]( std::string_view inpt, std::size_t expected, hdr_state expected_cs ) {
                page_selector cd{ hdr( inpt[0] ) };
                for ( std::size_t i = 1; i < inpt.size(); ++i )
                        cd.on_page( i, hdr( inpt[i] ) );
                auto [i, cs] = std::move( cd ).finish();
                EXPECT_EQ( i, expected ) << inpt;
                EXPECT_EQ( cs, expected_cs ) << inpt;
        };

        test_f( "AAAA", 3, hdr_state::A );
        test_f( "BBAA", 1, hdr_state::B );
        test_f( "BBBA", 2, hdr_state::B );
        test_f( "BBBB", 3, hdr_state::B );
        test_f( "CCBB", 1, hdr_state::C );
        test_f( "CCCC", 3, hdr_state::C );
        test_f( "AACC", 1, hdr_state::A );
}

TEST( cfg, zero_cell )
{
        std::byte tmp[cell_size] = { 0x00_b, 0x00_b, 0x00_b, 0x00_b };
        auto      r = deser_cell( std::span< std::byte, cell_size >{ tmp, cell_size } );
        EXPECT_FALSE( r.has_value() );

        EXPECT_TRUE( is_free_cell( 0x00 ) );
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

        test_fs( 0x7FFF'FFFF, 0xFFFF'FFFF, 0xFFFF'FFFF'FFFF'FFFF );
        test_fs( 0x1234'5678, 0x1234'5678, 0x9234'5678'1234'5678 );
        test_f(
            0x1234'5678,
            { 0x12_b, 0x56_b, 0xFF_b, 0xFF_b, 0x12_b, 0x56_b, 0xFF_b, 0xFF_b, 0xFF_b },
            0x1234'5678'0000'0002 );
        test_f( 0x1234'5678, { 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b }, 0x1234'5678'0000'0001 );
}

TEST( cfg, store )
{
        auto test_f = [&]< typename T >( uint32_t key, T value, std::size_t size ) {
                std::byte              buffer[42];
                std::span< std::byte > used_area;
                auto                   res = store_kval( key, value, buffer, used_area );
                EXPECT_EQ( res, result::SUCCESS );
                EXPECT_EQ( used_area.size(), size );
        };

        test_f( 0x0FFF'FFFF, 3.14F, 8 );
        test_f( 0x0FFF'FFFF, uint32_t{ 42 }, 8 );
        test_f( 0x0FFF'FFFF, uint64_t{ 0xFFFF'FFFF'FFFF'FFFF }, 16 );
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

TEST( cfg, loader )
{
        using kv = std::pair< uint32_t, std::vector< std::byte > >;

        auto test_f = [&]( std::vector< kv > data ) {
                std::map< uint32_t, std::vector< std::byte > > expected{ data.begin(), data.end() };
                for ( auto& [key, value] : expected )
                        while ( value.size() % cell_size != 0 )
                                value.push_back( 0x00_b );

                buffer_builder bb;
                for ( auto& [key, value] : data )
                        bb.add( key, std::move( value ) );
                auto buffer = std::move( bb ).build();

                auto key_check = []( auto&& ) {
                        return false;
                };
                std::map< uint32_t, std::vector< std::byte > > res;

                std::byte tmp[42];

                page_loader pl( buffer.size(), key_check, tmp );
                while ( !pl.errored() ) {
                        auto addr = pl.addr_to_read();
                        if ( addr == 0 && buffer.size() == 0 )
                                break;
                        pl.on_cell(
                            std::span< std::byte, cell_size >{ &buffer[addr], cell_size },
                            [&]( uint32_t key, std::span< std::byte > value ) {
                                    auto& vec = res[key] =
                                        std::vector< std::byte >{ value.begin(), value.end() };
                                    while ( vec.size() % cell_size != 0 )
                                            vec.push_back( 0x00_b );
                            } );
                        if ( addr == 0x00 )
                                break;
                }

                EXPECT_EQ( res, expected );
        };

        // XXX: the fact that we always have to padd this to multiple is a bit weird
        test_f( {} );
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
              { 0x2, { 0xFF_b, 0xFF_b, 1_b, 2_b, 3_b, 4_b, 5_b } },
              { 0x3, { 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b } },
              { 0x4, { 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b } } } );

        test_f(
            { { __LINE__, { 0xFF_b, 0xFF_b, 12_b, 3_b, 43_b, 43_b, 4_b, 3_b, 4_b, 34_b, 43_b } },
              { 0x3, { 0xFF_b, 0xFF_b } },
              { 0x4, { 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b, 0xFF_b } } } );
}

}  // namespace emlabcpp::cfg
