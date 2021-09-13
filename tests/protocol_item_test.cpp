#include "emlabcpp/iterators/convert.h"
#include "emlabcpp/protocol/item.h"
#include "util.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

template < protocol_endianess_enum Endianess, typename T >
struct valid_test_case : protocol_test_fixture
{
        static constexpr protocol_endianess_enum endianess = Endianess;
        using pitem                                        = protocol_item< T, endianess >;
        static_assert( protocol_item_check< pitem > );
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

                std::size_t used =
                    *pitem::serialize_at( bspan.template first< pitem::max_size >(), val );

                EXPECT_EQ( used, expected_buffer.size() );
                auto serialized = convert_view_n< int >( buffer.begin(), used );
                EXPECT_EQ( serialized, view{ expected_buffer } )
                    << std::hex  //
                    << "serialized : " << serialized << "\n"
                    << "expected   : " << convert_view< int >( expected_buffer ) << "\n";

                pitem::deserialize( view_n( buffer.begin(), used ) )
                    .match(
                        [&]( auto res ) {
                                EXPECT_EQ( *res.used, expected_buffer.size() );
                                EXPECT_EQ( res.val, val );
                        },
                        [&]( protocol_error_record ) {
                                FAIL();
                        } );
        }

        void generate_name( std::ostream& os ) const final
        {
                os << "test case for: " << pretty_name< T >() << ";"
                   << " " << Endianess << ";" << std::hex
                   << " expected binary data: " << convert_view< int >( expected_buffer ) << ";";
        }
};

template < protocol_endianess_enum Endianess, typename T >
std::function< protocol_test_fixture*() > make_valid_test_case( T val, std::vector< uint8_t > buff )
{
        return [=]() {
                return new valid_test_case< Endianess, T >( val, buff );
        };
}
template < protocol_endianess_enum Endianess, typename T >
std::function< protocol_test_fixture*() > make_specific_valid_test_case(
    typename protocol_item< T, Endianess >::value_type val,
    std::vector< uint8_t >                             buff )
{
        return [=]() {
                return new valid_test_case< Endianess, T >( val, buff );
        };
}

template < typename T >
struct invalid_test_case : protocol_test_fixture
{

        using pitem      = protocol_item< T, PROTOCOL_BIG_ENDIAN >;
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

                pitem::deserialize( view_n( tmp.begin(), inpt.size() ) )
                    .match(
                        [&]( auto ) {
                                FAIL();
                        },
                        [&]( protocol_error_record rec ) {
                                EXPECT_EQ( rec, expected_rec );
                        } );
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
make_invalid_test_case( std::vector< uint8_t > buff, protocol_error_record rec )
{
        return [=]() {
                return new invalid_test_case< T >( buff, rec );
        };
}

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
            make_invalid_test_case< int16_t >(
                { 255 }, protocol_error_record{ PROTOCOL_NS, LOWSIZE_ERR, 0 } ),
            make_invalid_test_case< uint32_t >(
                { 255, 255, 255 }, protocol_error_record{ PROTOCOL_NS, LOWSIZE_ERR, 0 } ),
            // std::array
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >(
                std::array< int16_t, 3 >{ -1, 1, 666 }, { 255, 255, 1, 0, 154, 2 } ),
            make_valid_test_case< PROTOCOL_BIG_ENDIAN >(
                std::array< int16_t, 3 >{ -1, 1, 666 }, { 255, 255, 0, 1, 2, 154 } ),
            make_invalid_test_case< std::array< uint32_t, 2 > >(
                { 12, 12, 12 }, protocol_error_record{ PROTOCOL_NS, LOWSIZE_ERR, 0 } ),
            make_invalid_test_case< std::array< uint32_t, 2 > >(
                { 12, 12, 12, 12 }, protocol_error_record{ PROTOCOL_NS, LOWSIZE_ERR, 4 } ),
            make_invalid_test_case< std::array< uint32_t, 2 > >(
                { 12, 12, 12, 12, 12, 12 }, protocol_error_record{ PROTOCOL_NS, LOWSIZE_ERR, 4 } ),
            // std::tuple
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >(
                std::tuple< uint8_t, int16_t, int8_t >{ 1u, 666u, -3u }, { 1, 154, 2, 253 } ),
            make_valid_test_case< PROTOCOL_BIG_ENDIAN >(
                std::tuple< uint8_t, int16_t, int8_t >{ 1u, 666u, -3u }, { 1, 2, 154, 253 } ),
            make_invalid_test_case< std::tuple< uint32_t > >(
                { 0, 0, 12 }, protocol_error_record{ PROTOCOL_NS, LOWSIZE_ERR, 0 } ),
            make_invalid_test_case< std::tuple< int8_t, uint16_t, int8_t, int8_t > >(
                { 0, 0, 12, 0 }, protocol_error_record{ PROTOCOL_NS, LOWSIZE_ERR, 4 } ),
            // std::variant
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >(
                std::variant< uint8_t, int16_t, uint16_t >{ int16_t{ -3 } }, { 1, 253, 255 } ),
            make_valid_test_case< PROTOCOL_BIG_ENDIAN >(
                std::variant< uint8_t, int16_t, uint16_t >{ int16_t{ -3 } }, { 1, 255, 253 } ),
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >(
                std::variant< uint8_t, int16_t, uint16_t >{ uint8_t{ 42 } }, { 0, 42 } ),
            make_invalid_test_case< std::variant< uint8_t, int16_t, uint16_t > >(
                { 3, 0, 0 }, protocol_error_record{ PROTOCOL_NS, UNDEFVAR_ERR, 0 } ),
            make_invalid_test_case< std::variant< uint8_t, int16_t, uint16_t > >(
                { 0 }, protocol_error_record{ PROTOCOL_NS, LOWSIZE_ERR, 1 } ),
            make_invalid_test_case< std::variant< uint8_t, int16_t, uint16_t > >(
                { 1, 0 }, protocol_error_record{ PROTOCOL_NS, LOWSIZE_ERR, 1 } ),
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
            make_invalid_test_case< std::bitset< 7 > >(
                {}, protocol_error_record{ PROTOCOL_NS, LOWSIZE_ERR, 0 } ),
            make_invalid_test_case< std::bitset< 9 > >(
                { 0x0 }, protocol_error_record{ PROTOCOL_NS, LOWSIZE_ERR, 0 } ),
            // protocol_sizeless_message
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >(
                *protocol_sizeless_message< 8 >::make( std::vector{ 1, 2, 3, 4, 5 } ),
                { 1, 2, 3, 4, 5 } ),
            make_valid_test_case< PROTOCOL_BIG_ENDIAN >(
                *protocol_sizeless_message< 8 >::make( std::vector{ 1, 2, 3, 4, 5 } ),
                { 1, 2, 3, 4, 5 } ),
            make_invalid_test_case< protocol_sizeless_message< 4 > >(
                { 0, 0, 0, 0, 0 }, protocol_error_record{ PROTOCOL_NS, BIGSIZE_ERR, 0 } ),
            // protocol_offset
            make_specific_valid_test_case< PROTOCOL_LITTLE_ENDIAN, protocol_offset< uint16_t, 0 > >(
                666u, { 154, 2 } ),
            make_specific_valid_test_case< PROTOCOL_BIG_ENDIAN, protocol_offset< uint16_t, 0 > >(
                666u, { 2, 154 } ),
            make_specific_valid_test_case< PROTOCOL_LITTLE_ENDIAN, protocol_offset< uint16_t, 4 > >(
                666u, { 158, 2 } ),
            make_specific_valid_test_case< PROTOCOL_BIG_ENDIAN, protocol_offset< uint16_t, 4 > >(
                666u, { 2, 158 } ),
            make_invalid_test_case< protocol_offset< uint16_t, 4 > >(
                { 255 }, protocol_error_record{ PROTOCOL_NS, LOWSIZE_ERR, 0 } ),
            // quantity
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >(
                tagged_quantity< struct offtag, uint16_t >{ 666u }, { 154, 2 } ),
            make_valid_test_case< PROTOCOL_BIG_ENDIAN >(
                tagged_quantity< struct offtag, uint16_t >{ 666u }, { 2, 154 } ),
            make_invalid_test_case< tagged_quantity< struct offtag, uint16_t > >(
                { 255 }, protocol_error_record{ PROTOCOL_NS, LOWSIZE_ERR, 0 } ),
            // bounded
            make_valid_test_case< PROTOCOL_LITTLE_ENDIAN >(
                bounded< uint16_t, 0, 1024u >::get< 666u >(), { 154, 2 } ),
            make_valid_test_case< PROTOCOL_BIG_ENDIAN >(
                bounded< uint16_t, 0, 1024u >::get< 666u >(), { 2, 154 } ),
            make_valid_test_case< PROTOCOL_BIG_ENDIAN >(
                bounded< int16_t, -1, 1 >::get< -1 >(), { 255, 255 } ),
            make_invalid_test_case< bounded< int16_t, -1, 1 > >(
                { 128 }, protocol_error_record{ PROTOCOL_NS, LOWSIZE_ERR, 0 } ),
            make_invalid_test_case< bounded< int16_t, -1, 1 > >(
                { 0, 128 }, protocol_error_record{ PROTOCOL_NS, BOUND_ERR, 0 } ),
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
            make_invalid_test_case< protocol_sized_buffer< uint16_t, uint16_t > >(
                { 0, 2, 2 }, protocol_error_record{ PROTOCOL_NS, LOWSIZE_ERR, 0 } ),
            make_invalid_test_case< protocol_sized_buffer< uint16_t, uint16_t > >(
                { 0, 1, 2, 2 }, protocol_error_record{ PROTOCOL_NS, LOWSIZE_ERR, 2 } )

        };

        exec_protocol_test_fixture_test( tests );
        return RUN_ALL_TESTS();
}
