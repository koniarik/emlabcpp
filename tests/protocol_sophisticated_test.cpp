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
///

#include "emlabcpp/convert_view.h"
#include "emlabcpp/match.h"
#include "emlabcpp/protocol/command_group.h"
#include "emlabcpp/protocol/handler.h"
#include "emlabcpp/protocol/streams.h"
#include "emlabcpp/protocol/tuple.h"
#include "util/util.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

enum ids : uint8_t
{
        FOO = 12,
        WOO = 1,
};

struct simple_group : protocol::command_group< std::endian::big >::with_commands<
                          protocol::command< FOO >::with_args< uint32_t, uint32_t >,
                          protocol::command< WOO >::with_args< std::array< uint8_t, 8 > > >
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
  : protocol::command_group< std::endian::big >::with_commands<
        protocol::command< CA >::with_args< int >,
        protocol::command< CB >,
        protocol::command< CC >::with_args< std::array< uint16_t, 3 > >,
        protocol::command<
            CD >::with_args< std::tuple< uint32_t, uint8_t >, int16_t, uint32_t, uint8_t, uint8_t >,
        protocol::command< CE >::with_args< std::variant< uint32_t, std::bitset< 13 > > >,
        protocol::command< CF >::with_args< uint32_t, protocol::sizeless_message< 16 > >,
        protocol::command< CG >::with_args< uint32_t, protocol::value_offset< uint8_t, 2 > >,
        protocol::command< CH >::with_args< test_quantity, uint16_t >,
        protocol::command< CI >::with_args<
            protocol::sized_buffer< uint8_t, protocol::sizeless_message< 8 > >,
            uint32_t >,
        protocol::command< CJ >::with_args< protocol::group< uint32_t, uint8_t > > >
{
};

struct test_tuple
  : protocol::tuple<
        std::endian::big >::with_items< uint32_t, uint16_t, std::bitset< 13 >, uint32_t >
{
};

template < auto ID, typename T >
struct simple_tag_struct
{
        static constexpr auto id = ID;

        T value;

        friend auto operator<=>(
            simple_tag_struct< ID, T > const&,
            simple_tag_struct< ID, T > const& ) = default;
};

using simple_ca = simple_tag_struct< CA, float >;
using simple_cb = simple_tag_struct< CB, uint32_t >;
using simple_cc = simple_tag_struct< CC, int16_t >;

using simple_tag_group     = protocol::tag_group< simple_ca, simple_cb, simple_cc >;
using simple_tag_group_var = typename protocol::traits_for< simple_tag_group >::value_type;

template < typename Group >
struct valid_test_case : protocol_test_fixture
{
        using handler      = protocol::handler< Group >;
        using value_type   = typename handler::value_type;
        using message_type = typename handler::message_type;

        value_type               val;
        std::vector< std::byte > expected_buffer;

        valid_test_case( value_type v, std::vector< std::byte > buff )
          : val( std::move( v ) )
          , expected_buffer( std::move( buff ) )
        {
        }

        void TestBody() final
        {
                auto       msg      = handler::serialize( val );
                bool const is_equal = equal( msg, expected_buffer );
                EXPECT_TRUE( is_equal ) << "msg: " << msg << "\n"
                                        << "expected: " << message_type( expected_buffer ) << "\n";

                match(
                    handler::extract( msg ),
                    [&]( value_type const& var ) {
                            EXPECT_EQ( var, val );
                    },
                    [&]( auto rec ) {
                            FAIL() << rec;
                    } );
        }

        void generate_name( std::ostream& os ) const final
        {
                os << "test case for: " << pretty_name< Group >()
                   << "; msg: " << convert_view< int >( expected_buffer );
        }
};

template < typename Group >
std::function< protocol_test_fixture*() > make_valid_test_case(
    typename protocol::traits_for< Group >::value_type val,
    std::vector< int > const&                          buff )
{
        return [=]() {
                auto cview = convert_view< std::byte >( buff );
                return new valid_test_case< Group >(
                    val, std::vector< std::byte >{ cview.begin(), cview.end() } );
        };
}

void protocol_sophisticated_tests()
{
        std::vector< std::function< protocol_test_fixture*() > > const tests = {
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
                    666u, protocol::sizeless_message< 16 >( 1, 2, 3, 4, 5, 6 ) ),
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
                    protocol::sizeless_message< 8 >( 1, 2, 3, 4, 5 ), 39439483u ),
                { 0, 23, 5, 1, 2, 3, 4, 5, 2, 89, 204, 123 } ),
            make_valid_test_case< complex_group >(
                complex_group::make_val< CJ >( static_cast< uint32_t >( 666 ) ),
                { 0, 24, 0, 0, 2, 154 } ),
            make_valid_test_case< complex_group >(
                complex_group::make_val< CJ >( static_cast< uint8_t >( 42 ) ), { 0, 24, 42 } ),
            make_valid_test_case< test_tuple >(
                test_tuple::make_val( 23657453, 666, std::bitset< 13 >{ 42 }, 6634343 ),
                { 1, 104, 251, 237, 2, 154, 42, 0, 0, 101, 59, 103 } ),
            make_valid_test_case< simple_tag_group >(
                simple_tag_group_var{ simple_ca{ 0.1f } }, { 0, 0, 61, 204, 204, 205 } ),
            make_valid_test_case< simple_tag_group >(
                simple_tag_group_var{ simple_cb{ 666 } }, { 0, 1, 0, 0, 2, 154 } ),

        };

        exec_protocol_test_fixture_test( tests );
}
}  // namespace emlabcpp
