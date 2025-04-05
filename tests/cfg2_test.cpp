
#include "emlabcpp/experimental/cfg2/handler.h"

#include <gtest/gtest.h>

namespace emlabcpp::cfg
{

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

TEST( cfg, seq )
{
        auto test_f = [&]( std::string_view inpt, std::size_t expected, hdr_state expected_cs ) {
                hdr_selector cd{ hdr( inpt[0] ) };
                for ( std::size_t i = 1; i < inpt.size(); ++i )
                        cd.on_cell( i, hdr( inpt[i] ) );
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
        auto tmp = deser_cell( 0x00 );
        EXPECT_FALSE( tmp.has_value() );
        EXPECT_TRUE( is_free_cell( 0x00 ) );
}

TEST( cfg, cell )
{
        auto test_f =
            [&]( uint32_t key, std::vector< uint64_t > value, std::vector< uint64_t > expected ) {
                    auto res = ser_cell( key, value );
                    EXPECT_TRUE( res.has_value() );
                    std::vector< uint64_t > tmp = { res->cell };
                    tmp.insert( tmp.end(), res->extra.begin(), res->extra.end() );
                    EXPECT_EQ( tmp, expected );

                    assert( !value.empty() );
                    std::optional< load_res > lr = deser_cell( res->cell );
                    EXPECT_EQ( lr->key, key );
                    if ( expected.size() > 1 ) {
                            EXPECT_TRUE( lr->is_seq );
                            EXPECT_EQ( lr->val, expected.size() - 1 );
                    } else {
                            EXPECT_FALSE( lr->is_seq );
                            EXPECT_EQ( lr->val, value[0] );
                    }
            };

        auto test_fs = [&]( uint32_t key, uint32_t value, uint64_t expected ) {
                test_f( key, { value }, { expected } );
        };

        test_fs( 0x7FFF'FFFF, 0xFFFF'FFFF, 0xFFFF'FFFF'FFFF'FFFF );
        test_fs( 0x1234'5678, 0x1234'5678, 0x9234'5678'1234'5678 );
        test_f(
            0x1234'5678,
            { 0x1234'5678, 0xFFFF'FFFF },
            { 0x1234'5678'0000'0002, 0x1234'5678, 0xFFFF'FFFF } );
        test_f(
            0x1234'5678,
            { 0xFFFF'FFFF'FFFF'FFFF },
            { 0x1234'5678'0000'0001, 0xFFFF'FFFF'FFFF'FFFF } );
}

struct buffer_builder
{
        auto& add( uint32_t key, std::vector< uint64_t > value ) &
        {
                auto res = ser_cell( key, value );
                EXPECT_TRUE( res.has_value() );
                buffer_.insert( buffer_.end(), res->extra.rbegin(), res->extra.rend() );
                buffer_.emplace_back( res->cell );
                return *this;
        }

        auto add( uint32_t key, std::vector< uint64_t > value ) &&
        {
                this->add( key, std::move( value ) );
                return std::move( *this );
        }

        auto build() &&
        {
                return std::move( buffer_ );
        }

        std::vector< uint64_t > buffer_;
};

TEST( cfg, loader )
{
        using kv = std::pair< uint32_t, std::vector< uint64_t > >;

        auto test_f = [&]( std::vector< kv > data ) {
                std::map< uint64_t, std::vector< uint64_t > > expected{ data.begin(), data.end() };

                buffer_builder bb;
                for ( auto& [key, value] : data )
                        bb.add( key, std::move( value ) );
                auto buffer = std::move( bb ).build();

                auto key_check = []( auto&& ) {
                        return false;
                };
                std::map< uint64_t, std::vector< uint64_t > > res;
                uint64_t                                      tmp[42];
                page_loader pl( buffer.size() * sizeof( uint64_t ), key_check, tmp );
                while ( !pl.errored() ) {
                        auto addr = pl.addr_to_read();
                        if ( addr == 0 && buffer.size() == 0 )
                                break;
                        pl.on_cell(
                            buffer[addr / sizeof( uint64_t )],
                            [&]( uint32_t key, std::span< uint64_t > value ) {
                                    res[key] =
                                        std::vector< uint64_t >{ value.begin(), value.end() };
                            } );
                        if ( addr == 0x00 )
                                break;
                }

                EXPECT_EQ( res, expected );
        };

        test_f(
            { { 0x1, { 0xFFFF'FFFF } },
              { 0x2, { 0xFFFF'FFFF } },
              { 0x3, { 0xFFFF'FFFF } },
              { 0x4, { 0xFFFF'FFFF } } } );

        test_f(
            { { 0x1, { 0xFFFF'FFFF } },
              { 0x2, { 0xFFFF'FFFF } },
              { 0x3, { 0xFFFF'FFFF } },
              { 0x4, { 0xFFFF'FFFF, 0xFFFF'FFFF } } } );

        test_f(
            { { 0x1, { 0xFFFF'FFFF } },
              { 0x2, { 0xFFFF'FFFF, 1, 2, 3, 4, 5 } },
              { 0x3, { 0xFFFF'FFFF } },
              { 0x4, { 0xFFFF'FFFF, 0xFFFF'FFFF, 0xFFFF'FFFF } } } );

        test_f(
            { { 0x1, { 0xFFFF'FFFF, 12, 3, 4, 5, 453, 43, 43, 43, 4, 3, 4, 343, 43 } },
              { 0x3, { 0xFFFF'FFFF } },
              { 0x4, { 0xFFFF'FFFF, 0xFFFF'FFFF, 0xFFFF'FFFF } } } );

        test_f( { { 0x1, { 0xFFFF'FFFF, 12, 3, 4, 5, 453, 43, 43, 43, 4, 3, 4, 343, 43 } } } );
        test_f( { { 0x1, { 0xFFFF'FFFF } } } );
        test_f( {} );
}

}  // namespace emlabcpp::cfg
