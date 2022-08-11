// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//
//  Copyright © 2022 Jan Veverak Koniarik
//  This file is part of project: emlabcpp
//
#include "emlabcpp/iterators/convert.h"
#include "emlabcpp/protocol/def.h"
#include "emlabcpp/protocol/streams.h"
#include "util.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

template < protocol_endianess_enum Endianess, typename T >
struct valid_test_case : protocol_test_fixture
{
        static constexpr protocol_endianess_enum endianess = Endianess;
        using pitem                                        = protocol_def< T, endianess >;
        static_assert( protocol_def_check< pitem > );
        using value_type = typename pitem::value_type;

        value_type             val;
        std::vector< uint8_t > expected_buffer;

        valid_test_case( value_type v, std::vector< uint8_t > buff )
          : val( std::move( v ) )
          , expected_buffer( std::move( buff ) )
        {
        }

        void TestBody() final
        {
                std::array< uint8_t, pitem::max_size > buffer{};
                std::span                              bspan{ buffer };

                bounded used =
                    pitem::serialize_at( bspan.template first< pitem::max_size >(), val );

                EXPECT_EQ( *used, expected_buffer.size() );
                auto serialized = convert_view_n< int >( buffer.begin(), *used );
                EXPECT_EQ( serialized, view{ expected_buffer } )
                    << std::hex  //
                    << "serialized      : " << serialized << "\n"
                    << "serialized (bin): "
                    << convert_view_n< std::bitset< 8 > >( buffer.begin(), *used ) << "\n"
                    << "expected        : " << convert_view< int >( expected_buffer ) << "\n"
                    << "expected   (bin): " << convert_view< std::bitset< 8 > >( expected_buffer )
                    << "\n";

                auto [pused, res] = pitem::deserialize(
                    *bounded_view< const uint8_t*, typename pitem::size_type >::make(
                        view_n( buffer.begin(), *used ) ) );
                if ( std::holds_alternative< const protocol_mark* >( res ) ) {
                        FAIL() << *std::get< 1 >( res );
                } else {
                        auto rval = std::get< 0 >( res );
                        EXPECT_EQ( rval, val );
                }
                EXPECT_EQ( pused, expected_buffer.size() );
        }

        void generate_name( std::ostream& os ) const final
        {
                os << "test case for: " << pretty_name< T >() << ";"
                   << " " << Endianess << ";" << std::hex
                   << " expected binary data: " << convert_view< int >( expected_buffer ) << ";";
        }
};

template < protocol_endianess_enum Endianess, typename T >
std::function< protocol_test_fixture*() >
make_valid_test_case( T val, const std::vector< uint8_t >& buff )
{
        return [=]() {
                return new valid_test_case< Endianess, T >( val, buff );
        };
}
template < protocol_endianess_enum Endianess, typename T >
std::function< protocol_test_fixture*() > make_specific_valid_test_case(
    typename protocol_def< T, Endianess >::value_type val,
    const std::vector< uint8_t >&                     buff )
{
        return [=]() {
                return new valid_test_case< Endianess, T >( val, buff );
        };
}

template < typename T >
struct invalid_test_case : protocol_test_fixture
{

        using pitem      = protocol_def< T, PROTOCOL_BIG_ENDIAN >;
        using value_type = T;

        std::vector< uint8_t > inpt;
        protocol_error_record  expected_rec;

        invalid_test_case( std::vector< uint8_t > buff, protocol_error_record rec )
          : inpt( std::move( buff ) )
          , expected_rec( rec )
        {
        }

        void TestBody() final
        {
                std::array< uint8_t, 2 * pitem::max_size > tmp{};
                ASSERT_LE( inpt.size(), tmp.size() );
                std::copy( inpt.begin(), inpt.end(), tmp.begin() );

                auto opt_view = bounded_view< const uint8_t*, typename pitem::size_type >::make(
                    view_n( tmp.begin(), inpt.size() ) );
                EXPECT_TRUE( opt_view );

                auto [used, res] = pitem::deserialize( *opt_view );
                if ( std::holds_alternative< const protocol_mark* >( res ) ) {
                        EXPECT_EQ( expected_rec.mark, *std::get< 1 >( res ) );
                        EXPECT_EQ( expected_rec.offset, used );
                } else {
                        FAIL() << "deserialization passed";
                }
        }

        void generate_name( std::ostream& os ) const final
        {
                os << "invalid test for: " << pretty_name< T >() << ";"
                   << " input: " << convert_view< int >( inpt ) << ";"
                   << " expected error: " << expected_rec << ";";
        }
};

template < typename T >
std::function< protocol_test_fixture*() >
make_invalid_test_case( const std::vector< uint8_t >& buff, const protocol_error_record& rec )
{
        return [=]() {
                return new invalid_test_case< T >( buff, rec );
        };
}

using variable_size_type = static_vector< uint8_t, 7 >;
const variable_size_type VARIABLE_VAL_1{ std::array< uint8_t, 3 >{ 1, 2, 3 } };
const variable_size_type VARIABLE_VAL_2{ std::array< uint8_t, 7 >{ 1, 2, 3, 4, 5, 6, 7 } };
const variable_size_type VARIABLE_VAL_3{ std::array< uint8_t, 0 >{} };

int main( int argc, char** argv )
{
        testing::InitGoogleTest( &argc, argv );

        std::vector< std::function< protocol_test_fixture*() > > tests = {
            // basic types
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >( uint8_t{ 42 }, { 42 } ),
            make_valid_test_case< PROTOCOL_BIG_ENDIAN >( uint8_t{ 42 }, { 42 } ),
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >( uint16_t{ 42 }, { 42, 0 } ),
            make_valid_test_case< PROTOCOL_BIG_ENDIAN >( uint16_t{ 42 }, { 0, 42 } ),
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >( uint16_t{ 666 }, { 154, 2 } ),
            make_valid_test_case< PROTOCOL_BIG_ENDIAN >( uint16_t{ 666 }, { 2, 154 } ),
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >( int32_t{ -1 }, { 255, 255, 255, 255 } ),
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >( 1.f, { 0, 0, 128, 63 } ),
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >( -1.f, { 0, 0, 128, 191 } ),
            // std::array
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >(
                std::array< int16_t, 3 >{ -1, 1, 666 }, { 255, 255, 1, 0, 154, 2 } ),
            make_valid_test_case< PROTOCOL_BIG_ENDIAN >(
                std::array< int16_t, 3 >{ -1, 1, 666 }, { 255, 255, 0, 1, 2, 154 } ),
            // std::tuple
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >(
                std::tuple< uint8_t, int16_t, int8_t >{ 1u, 666u, -3 }, { 1, 154, 2, 253 } ),
            make_valid_test_case< PROTOCOL_BIG_ENDIAN >(
                std::tuple< uint8_t, int16_t, int8_t >{ 1u, 666u, -3 }, { 1, 2, 154, 253 } ),
            // std::variant
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >(
                std::variant< uint8_t, int16_t, uint16_t >{ int16_t{ -3 } }, { 1, 253, 255 } ),
            make_valid_test_case< PROTOCOL_BIG_ENDIAN >(
                std::variant< uint8_t, int16_t, uint16_t >{ int16_t{ -3 } }, { 1, 255, 253 } ),
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >(
                std::variant< uint8_t, int16_t, uint16_t >{ uint8_t{ 42 } }, { 0, 42 } ),
            make_invalid_test_case< std::variant< uint8_t, int16_t, uint16_t > >(
                { 3, 0, 0 }, protocol_error_record{ UNDEFVAR_ERR, 0 } ),
            make_invalid_test_case< std::variant< uint8_t, int16_t, uint16_t > >(
                { 1, 0 }, protocol_error_record{ SIZE_ERR, 0 } ),
            // std::bitset
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >(
                std::bitset< 3 >{ 0b00000111 }, { 0b00000111 } ),
            make_valid_test_case< PROTOCOL_BIG_ENDIAN >(
                std::bitset< 3 >{ 0b00000111 }, { 0b00000111 } ),
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >(
                std::bitset< 15 >{ 0xFF55 }, { 0b01111111, 0x55 } ),
            make_valid_test_case< PROTOCOL_BIG_ENDIAN >(
                std::bitset< 15 >{ 0xFF55 }, { 0x55, 0b01111111 } ),
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >(
                std::bitset< 16 >{ 0xFFFF }, { 0xFF, 0xFF } ),
            // protocol_sizeless_message
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >(
                *protocol_sizeless_message< 8 >::make( std::vector{ 1, 2, 3, 4, 5 } ),
                { 1, 2, 3, 4, 5 } ),
            make_valid_test_case< PROTOCOL_BIG_ENDIAN >(
                *protocol_sizeless_message< 8 >::make( std::vector{ 1, 2, 3, 4, 5 } ),
                { 1, 2, 3, 4, 5 } ),
            // protocol_offset
            make_specific_valid_test_case< PROTOCOL_LITTLE_ENDIAN, protocol_offset< uint16_t, 0 > >(
                666u, { 154, 2 } ),
            make_specific_valid_test_case< PROTOCOL_BIG_ENDIAN, protocol_offset< uint16_t, 0 > >(
                666u, { 2, 154 } ),
            make_specific_valid_test_case< PROTOCOL_LITTLE_ENDIAN, protocol_offset< uint16_t, 4 > >(
                666u, { 158, 2 } ),
            make_specific_valid_test_case< PROTOCOL_BIG_ENDIAN, protocol_offset< uint16_t, 4 > >(
                666u, { 2, 158 } ),
            // quantity
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >(
                tagged_quantity< struct offtag, uint16_t >{ 666u }, { 154, 2 } ),
            make_valid_test_case< PROTOCOL_BIG_ENDIAN >(
                tagged_quantity< struct offtag, uint16_t >{ 666u }, { 2, 154 } ),
            // bounded
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >(
                bounded< uint16_t, 0, 1024u >::get< 666u >(), { 154, 2 } ),
            make_valid_test_case< PROTOCOL_BIG_ENDIAN >(
                bounded< uint16_t, 0, 1024u >::get< 666u >(), { 2, 154 } ),
            make_valid_test_case< PROTOCOL_BIG_ENDIAN >(
                bounded< int16_t, -1, 1 >::get< -1 >(), { 255, 255 } ),
            make_invalid_test_case< bounded< int16_t, -1, 1 > >(
                { 0, 128 }, protocol_error_record{ BOUNDS_ERR, 0 } ),
            // sized_buffer
            make_specific_valid_test_case<
                PROTOCOL_LITTLE_ENDIAN,
                protocol_sized_buffer< uint16_t, protocol_sizeless_message< 12 > > >(
                *protocol_sizeless_message< 12 >::make( std::vector{ 1, 2, 3, 4, 5, 6, 7 } ),
                { 7, 0, 1, 2, 3, 4, 5, 6, 7 } ),
            make_specific_valid_test_case<
                PROTOCOL_BIG_ENDIAN,
                protocol_sized_buffer< uint16_t, protocol_sizeless_message< 12 > > >(
                *protocol_sizeless_message< 12 >::make( std::vector{ 1, 2, 3, 4, 5, 6, 7 } ),
                { 0, 7, 1, 2, 3, 4, 5, 6, 7 } ),
            make_specific_valid_test_case<
                PROTOCOL_LITTLE_ENDIAN,
                protocol_sized_buffer< uint16_t, uint16_t > >( 666u, { 2, 0, 154, 2 } ),
            make_specific_valid_test_case<
                PROTOCOL_BIG_ENDIAN,
                protocol_sized_buffer< uint16_t, uint16_t > >( 666u, { 0, 2, 2, 154 } ),
            make_specific_valid_test_case<
                PROTOCOL_BIG_ENDIAN,
                protocol_sized_buffer< protocol_offset< uint16_t, 2 >, uint16_t > >(
                666u, { 0, 4, 2, 154 } ),
            make_invalid_test_case< protocol_sized_buffer< uint16_t, uint16_t > >(
                { 0, 1, 2, 2 }, protocol_error_record{ SIZE_ERR, 2 } ),
            make_invalid_test_case< protocol_sized_buffer< uint16_t, uint16_t > >(
                { 0, 5, 2, 2 }, protocol_error_record{ SIZE_ERR, 2 } ),
            // tag
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >( tag< 666u >{}, { 154, 2, 0, 0 } ),
            make_valid_test_case< PROTOCOL_BIG_ENDIAN >( tag< 666u >{}, { 0, 0, 2, 154 } ),
            make_invalid_test_case< tag< 666u > >(
                { 0, 0, 2, 152 }, protocol_error_record{ BADVAL_ERR, 0 } ),
            // group is tested as part of command group
            // endianess change
            make_specific_valid_test_case<
                PROTOCOL_LITTLE_ENDIAN,
                protocol_endianess< PROTOCOL_BIG_ENDIAN, uint16_t > >( uint16_t{ 42 }, { 0, 42 } ),
            // static_vector
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >(
                static_vector< int8_t, 9 >( std::array< int8_t, 3 >{ -1, 0, 42 } ),
                { 3, 0, 255, 0, 42 } ),
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >(
                static_vector< int16_t, 9 >( std::array< int16_t, 4 >{ -1, 0, 666, 42 } ),
                { 4, 0, 255, 255, 0, 0, 154, 2, 42, 0 } ),
            make_valid_test_case< PROTOCOL_BIG_ENDIAN >(
                static_vector< int16_t, 9 >( std::array< int16_t, 4 >{ -1, 0, 666, 42 } ),
                { 0, 4, 255, 255, 0, 0, 2, 154, 0, 42 } ),
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >(
                static_vector< int16_t, 9 >(
                    std::array< int16_t, 9 >{ 1, 2, 3, 4, 5, 6, 7, 8, 9 } ),
                { 9, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0, 9, 0 } ),
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >(
                static_vector< int16_t, 9 >(), { 0, 0 } ),
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >(
                static_vector< variable_size_type, 4 >( std::array< variable_size_type, 3 >{
                    VARIABLE_VAL_1, VARIABLE_VAL_2, VARIABLE_VAL_3 } ),
                { 3, 0, 3, 0, 1, 2, 3, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 0 } ),
            make_invalid_test_case< static_vector< int16_t, 9 > >(
                { 1, 0 }, protocol_error_record{ SIZE_ERR, 2 } ),
            make_invalid_test_case< static_vector< int16_t, 9 > >(
                { 4, 0, 0, 0, 0, 0, 0 }, protocol_error_record{ SIZE_ERR, 6 } ),
            // optional
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >( std::optional< int32_t >{}, { 0 } ),
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >(
                std::optional< int32_t >{ 42u }, { 1, 0x2a, 0, 0, 0 } ),
            make_invalid_test_case< std::optional< tag< 666u > > >(
                { 1, 0 }, protocol_error_record{ BADVAL_ERR, 1 } ),
            make_invalid_test_case< std::optional< tag< 666u > > >(
                { 1, 0, 0, 2, 152 }, protocol_error_record{ BADVAL_ERR, 1 } ) };

        exec_protocol_test_fixture_test( tests );
        return RUN_ALL_TESTS();
}
