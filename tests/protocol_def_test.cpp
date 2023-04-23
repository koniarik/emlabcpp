///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
/// and associated documentation files (the "Software"), to deal in the Software without
/// restriction, including without limitation the rights to use, copy, modify, merge, publish,
/// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or
/// substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
/// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
/// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#include "emlabcpp/convert_view.h"
#include "emlabcpp/protocol/converter.h"
#include "emlabcpp/protocol/streams.h"
#include "util.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

template < std::endian Endianess, typename T >
struct valid_test_case : protocol_test_fixture
{
        static constexpr std::endian endianess = Endianess;
        using pitem                            = protocol::converter_for< T, endianess >;
        static_assert( protocol::converter_check< pitem > );
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
                std::array< std::byte, pitem::max_size > buffer{};
                const std::span                          bspan{ buffer };

                const bounded used =
                    pitem::serialize_at( bspan.template first< pitem::max_size >(), val );

                EXPECT_EQ( *used, expected_buffer.size() );
                auto serialized = convert_view_n< int >( buffer.begin(), *used );
                EXPECT_EQ( serialized, view{ expected_buffer } )
                    << std::hex  //
                    << "serialized      : " << serialized << "\n"
                    << "expected        : " << convert_view< int >( expected_buffer ) << "\n";

                value_type item;
                auto       sres = pitem::deserialize(
                    std::span< const std::byte >{ buffer.begin(), *used }, item );

                if ( sres.has_error() ) {
                        FAIL() << *sres.get_error();
                        return;
                }
                EXPECT_EQ( item, val );
                EXPECT_EQ( sres.used, expected_buffer.size() );
        }

        void generate_name( std::ostream& os ) const final
        {
                pretty_stream_write( os, "test case for: ", pretty_name< T >(), ";" );
                pretty_stream_write( os, " ", Endianess, ";" );
                pretty_stream_write(
                    os, " expected binary data: ", convert_view< int >( expected_buffer ), ";" );
        }
};

template < std::endian Endianess, typename T >
std::function< protocol_test_fixture*() >
make_valid_test_case( T val, const std::vector< uint8_t >& buff )
{
        return [=]() {
                return new valid_test_case< Endianess, T >( val, buff );
        };
}
template < std::endian Endianess, typename T >
std::function< protocol_test_fixture*() > make_specific_valid_test_case(
    typename protocol::converter_for< T, Endianess >::value_type val,
    const std::vector< uint8_t >&                                buff )
{
        return [=]() {
                return new valid_test_case< Endianess, T >( val, buff );
        };
}

template < typename T >
struct invalid_test_case : protocol_test_fixture
{

        using pitem      = protocol::converter_for< T, std::endian::big >;
        using value_type = typename pitem::value_type;

        std::vector< std::byte > inpt;
        protocol::error_record   expected_rec;

        invalid_test_case( std::vector< std::byte > buff, protocol::error_record rec )
          : inpt( std::move( buff ) )
          , expected_rec( rec )
        {
        }

        void TestBody() final
        {
                std::array< std::byte, 2 * pitem::max_size > tmp{};
                ASSERT_LE( inpt.size(), tmp.size() );
                copy( inpt, tmp.begin() );

                auto opt_view = bounded_view< const std::byte*, typename pitem::size_type >::make(
                    view_n( tmp.begin(), inpt.size() ) );
                EXPECT_TRUE( opt_view );

                value_type item;
                auto [used, err] = pitem::deserialize( *opt_view, item );
                if ( err != nullptr ) {
                        EXPECT_EQ( expected_rec.error_mark, *err );
                        EXPECT_EQ( expected_rec.offset, used );
                } else {
                        FAIL() << "deserialization passed";
                }
        }

        void generate_name( std::ostream& os ) const final
        {
                pretty_stream_write( os, "invalid test for: ", pretty_name< T >(), ";" );
                pretty_stream_write( os, " input: ", inpt, ";" );
                pretty_stream_write( os, " expected error: ", expected_rec, ";" );
        }
};

template < typename T >
std::function< protocol_test_fixture*() >
make_invalid_test_case( const std::vector< uint8_t >& buff, const protocol::error_record& rec )
{
        auto cview = convert_view< std::byte >( buff );
        return [=]() {
                return new invalid_test_case< T >(
                    std::vector< std::byte >{ cview.begin(), cview.end() }, rec );
        };
}

using variable_size_type = static_vector< uint8_t, 7 >;
const variable_size_type VARIABLE_VAL_1{ std::array< uint8_t, 3 >{ 1, 2, 3 } };
const variable_size_type VARIABLE_VAL_2{ std::array< uint8_t, 7 >{ 1, 2, 3, 4, 5, 6, 7 } };
const variable_size_type VARIABLE_VAL_3{ std::array< uint8_t, 0 >{} };

template < typename T >
struct simple_struct
{
        T value;

        friend auto operator<=>( const simple_struct< T >&, const simple_struct< T >& ) = default;
};

int main( int argc, char** argv )
{
        testing::InitGoogleTest( &argc, argv );

        const std::vector< std::function< protocol_test_fixture*() > > tests = {
            // basic types
            make_valid_test_case< std::endian::little >( uint8_t{ 42 }, { 42 } ),
            make_valid_test_case< std::endian::big >( uint8_t{ 42 }, { 42 } ),
            make_valid_test_case< std::endian::little >( uint16_t{ 42 }, { 42, 0 } ),
            make_valid_test_case< std::endian::big >( uint16_t{ 42 }, { 0, 42 } ),
            make_valid_test_case< std::endian::little >( uint16_t{ 666 }, { 154, 2 } ),
            make_valid_test_case< std::endian::big >( uint16_t{ 666 }, { 2, 154 } ),
            make_valid_test_case< std::endian::little >( int32_t{ -1 }, { 255, 255, 255, 255 } ),
            make_valid_test_case< std::endian::little >( 1.f, { 0, 0, 128, 63 } ),
            make_valid_test_case< std::endian::little >( -1.f, { 0, 0, 128, 191 } ),
            // std::array
            make_valid_test_case< std::endian::little >(
                std::array< int16_t, 3 >{ -1, 1, 666 }, { 255, 255, 1, 0, 154, 2 } ),
            make_valid_test_case< std::endian::big >(
                std::array< int16_t, 3 >{ -1, 1, 666 }, { 255, 255, 0, 1, 2, 154 } ),
            // std::tuple
            make_valid_test_case< std::endian::little >(
                std::tuple< uint8_t, int16_t, int8_t >{ 1u, 666u, -3 }, { 1, 154, 2, 253 } ),
            make_valid_test_case< std::endian::big >(
                std::tuple< uint8_t, int16_t, int8_t >{ 1u, 666u, -3 }, { 1, 2, 154, 253 } ),
            // std::variant
            make_valid_test_case< std::endian::little >(
                std::variant< uint8_t, int16_t, uint16_t >{ int16_t{ -3 } }, { 1, 253, 255 } ),
            make_valid_test_case< std::endian::big >(
                std::variant< uint8_t, int16_t, uint16_t >{ int16_t{ -3 } }, { 1, 255, 253 } ),
            make_valid_test_case< std::endian::little >(
                std::variant< uint8_t, int16_t, uint16_t >{ uint8_t{ 42 } }, { 0, 42 } ),
            make_valid_test_case< std::endian::little >(
                std::variant< uint8_t, uint8_t, uint16_t >(
                    std::in_place_index< 1 >, uint8_t{ 42 } ),
                { 1, 42 } ),
            make_invalid_test_case< std::variant< uint8_t, int16_t, uint16_t > >(
                { 3, 0, 0 }, protocol::error_record{ protocol::UNDEFVAR_ERR, 0 } ),
            make_invalid_test_case< std::variant< uint8_t, int16_t, uint16_t > >(
                { 1, 0 }, protocol::error_record{ protocol::SIZE_ERR, 1 } ),
            // std::bitset
            make_valid_test_case< std::endian::little >(
                std::bitset< 3 >{ 0b00000111 }, { 0b00000111 } ),
            make_valid_test_case< std::endian::big >(
                std::bitset< 3 >{ 0b00000111 }, { 0b00000111 } ),
            make_valid_test_case< std::endian::little >(
                std::bitset< 15 >{ 0xFF55 }, { 0b01111111, 0x55 } ),
            make_valid_test_case< std::endian::big >(
                std::bitset< 15 >{ 0xFF55 }, { 0x55, 0b01111111 } ),
            make_valid_test_case< std::endian::little >(
                std::bitset< 16 >{ 0xFFFF }, { 0xFF, 0xFF } ),
            // protocol::sizeless_message
            make_valid_test_case< std::endian::little >(
                protocol::sizeless_message{ 1, 2, 3, 4, 5 }, { 1, 2, 3, 4, 5 } ),
            make_valid_test_case< std::endian::big >(
                protocol::sizeless_message< 8 >{ 1, 2, 3, 4, 5 }, { 1, 2, 3, 4, 5 } ),
            // protocol::message
            make_valid_test_case< std::endian::little >(
                protocol::message< 8 >{ 1, 2, 3, 4, 5 }, { 5, 0, 1, 2, 3, 4, 5 } ),
            make_valid_test_case< std::endian::big >(
                protocol::message< 8 >{ 1, 2, 3, 4, 5 }, { 0, 5, 1, 2, 3, 4, 5 } ),
            // protocol::value_offset
            make_specific_valid_test_case<
                std::endian::little,
                protocol::value_offset< uint16_t, 0 > >( 666u, { 154, 2 } ),
            make_specific_valid_test_case<
                std::endian::big,
                protocol::value_offset< uint16_t, 0 > >( 666u, { 2, 154 } ),
            make_specific_valid_test_case<
                std::endian::little,
                protocol::value_offset< uint16_t, 4 > >( 666u, { 158, 2 } ),
            make_specific_valid_test_case<
                std::endian::big,
                protocol::value_offset< uint16_t, 4 > >( 666u, { 2, 158 } ),
            // quantity
            make_valid_test_case< std::endian::little >(
                tagged_quantity< struct offtag, uint16_t >{ 666u }, { 154, 2 } ),
            make_valid_test_case< std::endian::big >(
                tagged_quantity< struct offtag, uint16_t >{ 666u }, { 2, 154 } ),
            // bounded
            make_valid_test_case< std::endian::little >(
                bounded< uint16_t, 0, 1024u >::get< 666u >(), { 154, 2 } ),
            make_valid_test_case< std::endian::big >(
                bounded< uint16_t, 0, 1024u >::get< 666u >(), { 2, 154 } ),
            make_valid_test_case< std::endian::big >(
                bounded< int16_t, -1, 1 >::get< -1 >(), { 255, 255 } ),
            make_invalid_test_case< bounded< int16_t, -1, 1 > >(
                { 0, 128 }, protocol::error_record{ protocol::BOUNDS_ERR, 0 } ),
            // sized_buffer
            make_specific_valid_test_case<
                std::endian::little,
                protocol::sized_buffer< uint16_t, protocol::sizeless_message< 12 > > >(
                protocol::sizeless_message< 12 >{ 1, 2, 3, 4, 5, 6, 7 },
                { 7, 0, 1, 2, 3, 4, 5, 6, 7 } ),
            make_specific_valid_test_case<
                std::endian::big,
                protocol::sized_buffer< uint16_t, protocol::sizeless_message< 12 > > >(
                protocol::sizeless_message< 12 >{ 1, 2, 3, 4, 5, 6, 7 },
                { 0, 7, 1, 2, 3, 4, 5, 6, 7 } ),
            make_specific_valid_test_case<
                std::endian::little,
                protocol::sized_buffer< uint16_t, uint16_t > >( 666u, { 2, 0, 154, 2 } ),
            make_specific_valid_test_case<
                std::endian::big,
                protocol::sized_buffer< uint16_t, uint16_t > >( 666u, { 0, 2, 2, 154 } ),
            make_specific_valid_test_case<
                std::endian::big,
                protocol::sized_buffer< protocol::value_offset< uint16_t, 2 >, uint16_t > >(
                666u, { 0, 4, 2, 154 } ),
            make_invalid_test_case< protocol::sized_buffer< uint16_t, uint16_t > >(
                { 0, 1, 2, 2 }, protocol::error_record{ protocol::SIZE_ERR, 2 } ),
            make_invalid_test_case< protocol::sized_buffer< uint16_t, uint16_t > >(
                { 0, 5, 2, 2 }, protocol::error_record{ protocol::SIZE_ERR, 2 } ),
            // tag
            make_valid_test_case< std::endian::little >( tag< 666u >{}, { 154, 2, 0, 0 } ),
            make_valid_test_case< std::endian::big >( tag< 666u >{}, { 0, 0, 2, 154 } ),
            make_invalid_test_case< tag< 666u > >(
                { 0, 0, 2, 152 }, protocol::error_record{ protocol::BADVAL_ERR, 0 } ),
            // group is tested as part of command group
            // endianess change
            make_specific_valid_test_case<
                std::endian::little,
                protocol::endianess_wrapper< std::endian::big, uint16_t > >(
                uint16_t{ 42 }, { 0, 42 } ),
            // static_vector
            make_valid_test_case< std::endian::little >(
                static_vector< int8_t, 9 >( std::array< int8_t, 3 >{ -1, 0, 42 } ),
                { 3, 0, 255, 0, 42 } ),
            make_valid_test_case< std::endian::little >(
                static_vector< int16_t, 9 >( std::array< int16_t, 4 >{ -1, 0, 666, 42 } ),
                { 4, 0, 255, 255, 0, 0, 154, 2, 42, 0 } ),
            make_valid_test_case< std::endian::big >(
                static_vector< int16_t, 9 >( std::array< int16_t, 4 >{ -1, 0, 666, 42 } ),
                { 0, 4, 255, 255, 0, 0, 2, 154, 0, 42 } ),
            make_valid_test_case< std::endian::little >(
                static_vector< int16_t, 9 >(
                    std::array< int16_t, 9 >{ 1, 2, 3, 4, 5, 6, 7, 8, 9 } ),
                { 9, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8, 0, 9, 0 } ),
            make_valid_test_case< std::endian::little >( static_vector< int16_t, 9 >(), { 0, 0 } ),
            make_valid_test_case< std::endian::little >(
                static_vector< variable_size_type, 4 >( std::array< variable_size_type, 3 >{
                    VARIABLE_VAL_1, VARIABLE_VAL_2, VARIABLE_VAL_3 } ),
                { 3, 0, 3, 0, 1, 2, 3, 7, 0, 1, 2, 3, 4, 5, 6, 7, 0, 0 } ),
            make_invalid_test_case< static_vector< int16_t, 9 > >(
                { 1, 0 }, protocol::error_record{ protocol::SIZE_ERR, 2 } ),
            make_invalid_test_case< static_vector< int16_t, 9 > >(
                { 4, 0, 0, 0, 0, 0, 0 }, protocol::error_record{ protocol::SIZE_ERR, 6 } ),
            // optional
            make_valid_test_case< std::endian::little >( std::optional< int32_t >{}, { 0 } ),
            make_valid_test_case< std::endian::little >(
                std::optional< int32_t >{ 42u }, { 1, 0x2a, 0, 0, 0 } ),
            make_invalid_test_case< std::optional< tag< 666u > > >(
                { 1, 0 }, protocol::error_record{ protocol::SIZE_ERR, 1 } ),
            make_invalid_test_case< std::optional< tag< 666u > > >(
                { 1, 0, 0, 2, 152 }, protocol::error_record{ protocol::BADVAL_ERR, 1 } ),
            // decompose
            make_valid_test_case< std::endian::big >(
                simple_struct< uint16_t >{ 666 }, { 2, 154 } ),
            make_valid_test_case< std::endian::little >(
                simple_struct< int32_t >{ -1 }, { 255, 255, 255, 255 } ),
            make_valid_test_case< std::endian::little >(
                simple_struct< float >{ 1.f }, { 0, 0, 128, 63 } ),
            make_valid_test_case< std::endian::little >(
                simple_struct< std::variant< uint8_t, int16_t, uint16_t > >{ uint8_t{ 42 } },
                { 0, 42 } ),
            make_invalid_test_case< simple_struct< std::variant< uint8_t, int16_t, uint16_t > > >(
                { 3, 0, 0 }, protocol::error_record{ protocol::UNDEFVAR_ERR, 0 } )

        };

        exec_protocol_test_fixture_test( tests );
        return RUN_ALL_TESTS();
}
