#include "emlabcpp/iterators/convert.h"
#include "emlabcpp/protocol/command_group.h"
#include "emlabcpp/protocol/handler.h"
#include "emlabcpp/protocol/tuple.h"
#include "util.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

enum ids : uint8_t
{
        FOO = 12,
        WOO = 1,
};

struct simple_group : protocol_command_group<
                          PROTOCOL_BIG_ENDIAN,
                          protocol_command< FOO >::with_args< uint32_t, uint32_t >,
                          protocol_command< WOO >::with_args< std::array< uint8_t, 8 > > >
{
};

enum compl_ids : uint16_t
{
        CA = 0,
        CB = 1,
        CC = 4,
        CD = 9,
        CE = 13,
        CF = 15,
        CG = 21,
        CH = 22,
        CI = 23,
        CJ = 24
};
using test_quantity = tagged_quantity< struct vtag, uint32_t >;
// TODO: test changed endinaess in one subitem
// TODO: add subprotocol
struct complex_group
  : protocol_command_group<
        PROTOCOL_BIG_ENDIAN,
        protocol_command< CA >::with_args< int >,
        protocol_command< CB >,
        protocol_command< CC >::with_args< std::array< uint16_t, 3 > >,
        protocol_command<
            CD >::with_args< std::tuple< uint32_t, uint8_t >, int16_t, uint32_t, uint8_t, uint8_t >,
        protocol_command< CE >::with_args< std::variant< uint32_t, std::bitset< 13 > > >,
        protocol_command< CF >::with_args< uint32_t, protocol_sizeless_message< 16 > >,
        protocol_command< CG >::with_args< uint32_t, protocol_offset< uint8_t, 2 > >,
        protocol_command< CH >::with_args< test_quantity, uint16_t >,
        protocol_command< CI >::
            with_args< protocol_sized_buffer< uint8_t, protocol_sizeless_message< 8 > >, uint32_t >,
        protocol_command< CJ >::with_args< protocol_group< uint32_t, uint8_t > > >
{
};

struct test_tuple
  : protocol_tuple< PROTOCOL_BIG_ENDIAN, uint32_t, uint16_t, std::bitset< 13 >, uint32_t >
{
};

template < typename Group >
struct valid_test_case : protocol_test_fixture
{
        using handler    = protocol_handler< Group >;
        using value_type = typename handler::value_type;

        value_type             val;
        std::vector< uint8_t > expected_buffer;

        valid_test_case( value_type v, std::vector< uint8_t > buff )
          : val( std::move( v ) )
          , expected_buffer( std::move( buff ) )
        {
        }

        void TestBody() final
        {
                auto msg      = handler::serialize( val );
                bool is_equal = equal( msg, expected_buffer );
                EXPECT_TRUE( is_equal )
                    << "msg: " << convert_view< int >( msg ) << "\n"
                    << "expected: " << convert_view< int >( expected_buffer ) << "\n";

                handler::extract( msg ).match(
                    [&]( auto var ) {
                            EXPECT_EQ( var, val );
                    },
                    [&]( auto ) {
                            FAIL();
                    } );
        }

        void generate_name( std::ostream& os ) const final
        {
                os << "test case for: " << pretty_name< Group >()
                   << "; msg: " << convert_view< int >( expected_buffer );
        }
};

template < typename Group >
std::function< protocol_test_fixture*() >
make_valid_test_case( typename Group::value_type val, std::vector< uint8_t > buff )
{
        return [=]() {
                return new valid_test_case< Group >( val, std::move( buff ) );
        };
}

int main( int argc, char** argv )
{
        testing::InitGoogleTest( &argc, argv );

        std::vector< std::function< protocol_test_fixture*() > > tests = {
            make_valid_test_case< simple_group >(
                simple_group::make_val< FOO >( 42u, 666u ), { 12, 0, 0, 0, 42, 0, 0, 2, 154 } ),
            make_valid_test_case< simple_group >(
                simple_group::make_val< WOO >( std::array< uint8_t, 8 >{ 5, 4, 3, 2, 7, 8 } ),
                { 1, 5, 4, 3, 2, 7, 8, 0, 0 } ),
            make_valid_test_case< complex_group >(
                complex_group::make_val< CA >( -1 ), { 0, 0, 255, 255, 255, 255 } ),
            make_valid_test_case< complex_group >( complex_group::make_val< CB >(), { 0, 1 } ),
            make_valid_test_case< complex_group >(
                complex_group::make_val< CC >( std::array< uint16_t, 3 >{ 2, 666, 42 } ),
                { 0, 4, 0, 2, 2, 154, 0, 42 } ),
            make_valid_test_case< complex_group >(
                complex_group::make_val< CD >(
                    std::tuple< uint32_t, uint8_t >{ 34567u, 255u },
                    static_cast< int16_t >( -666 ),
                    static_cast< uint32_t >( 666 ),
                    static_cast< uint8_t >( 42 ),
                    static_cast< uint8_t >( 9 ) ),
                { 0, 9, 0, 0, 135, 7, 255, 253, 102, 0, 0, 2, 154, 42, 9 } ),
            make_valid_test_case< complex_group >(
                complex_group::make_val< CE >( 66u ), { 0, 13, 0, 0, 0, 0, 66 } ),
            make_valid_test_case< complex_group >(
                complex_group::make_val< CE >( std::bitset< 13 >{ 42 } ), { 0, 13, 1, 42, 0 } ),
            make_valid_test_case< complex_group >(
                complex_group::make_val< CF >(
                    666u,
                    *protocol_sizeless_message< 16 >::make( std::vector{ 1, 2, 3, 4, 5, 6 } ) ),
                { 0, 15, 0, 0, 2, 154, 1, 2, 3, 4, 5, 6 } ),
            make_valid_test_case< complex_group >(
                complex_group::make_val< CG >( 23456u, static_cast< uint8_t >( 8 ) ),
                { 0, 21, 0, 0, 91, 160, 10 } ),
            make_valid_test_case< complex_group >(
                complex_group::make_val< CH >(
                    test_quantity{ 2348974582u }, static_cast< uint16_t >( 666u ) ),
                { 0, 22, 140, 2, 129, 246, 2, 154 } ),
            make_valid_test_case< complex_group >(
                complex_group::make_val< CI >(
                    *protocol_sizeless_message< 8 >::make( std::vector{ 1, 2, 3, 4, 5 } ),
                    39439483u ),
                { 0, 23, 5, 1, 2, 3, 4, 5, 2, 89, 204, 123 } ),
            make_valid_test_case< complex_group >(
                complex_group::make_val< CJ >( static_cast< uint32_t >( 666 ) ),
                { 0, 24, 0, 0, 2, 154 } ),
            make_valid_test_case< complex_group >(
                complex_group::make_val< CJ >( static_cast< uint8_t >( 42 ) ), { 0, 24, 42 } ),
            make_valid_test_case< test_tuple >(
                test_tuple::make_val( 23657453, 666, std::bitset< 13 >{ 42 }, 6634343 ),
                { 1, 104, 251, 237, 2, 154, 42, 0, 0, 101, 59, 103 } ) };

        exec_protocol_test_fixture_test( tests );
        return RUN_ALL_TESTS();
}
