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
#include "emlabcpp/protocol/register_handler.h"
#include "emlabcpp/protocol/register_map.h"
#include "emlabcpp/protocol/streams.h"
#include "util.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

enum test_keys
{
        FOO = 1,
        WOO = 4,
        TOO = 8,
        SOO = 9,
        KOO = 10
};

struct test_map : protocol_register_map<
                      PROTOCOL_BIG_ENDIAN,
                      protocol_reg< FOO, uint32_t >,
                      protocol_reg< WOO, uint32_t >,
                      protocol_reg< TOO, uint8_t >,
                      protocol_reg< SOO, bounded< uint8_t, 2, 4 > >,
                      protocol_reg< KOO, protocol_offset< uint32_t, 42 > > >
{
};

struct test_handler : protocol_register_handler< test_map >
{
};

template < test_keys Key >
struct valid_test_case : protocol_test_fixture
{
        using value_type = typename test_map::reg_value_type< Key >;
        using pitem = protocol_def< typename test_map::reg_def_type< Key >, PROTOCOL_BIG_ENDIAN >;
        using message_type                    = test_map::message_type;
        static constexpr std::size_t max_size = test_handler::max_size;

        value_type val;

        valid_test_case( value_type value )
          : val( value )
        {
        }

        void TestBody() final
        {
                test_map m;

                std::array< uint8_t, max_size > buffer;
                bounded                         used = pitem::serialize_at(
                    std::span< uint8_t, pitem::max_size >( buffer.begin(), pitem::max_size ), val );
                auto source_msg = *message_type::make( view_n( buffer.begin(), *used ) );
                test_handler::extract< Key >( source_msg )
                    .match(
                        [&]( auto val ) {
                                m.set_val< Key >( val );
                        },
                        [&]( auto err ) {
                                FAIL() << err;
                        } );

                value_type stored = m.get_val< Key >();

                EXPECT_EQ( val, stored );

                auto msg = test_handler::serialize< Key >( stored );
                EXPECT_EQ( source_msg, msg ) << m;
        }

        void generate_name( std::ostream& os ) const final
        {
                os << "test case for key: " << Key;
        }
};

template < test_keys Key >
std::function< protocol_test_fixture*() >
make_valid_test_case( typename test_map::reg_value_type< Key > val )
{
        return [=]() {
                return new valid_test_case< Key >( val );
        };
}

TEST( protocol_map, runtime_access )
{
        std::vector< test_keys > keys{ FOO, WOO, TOO, SOO, KOO };
        for ( auto [i, k] : enumerate( keys ) ) {
                EXPECT_EQ( test_map::register_key( *test_map::register_index::make( i ) ), k );
        }

        std::vector< std::size_t > sizes{ 4, 4, 1, 1, 4 };
        for ( auto [i, s] : enumerate( sizes ) ) {
                EXPECT_EQ( test_map::register_size( *test_map::register_index::make( i ) ), s );
        }
}

int main( int argc, char** argv )
{
        testing::InitGoogleTest( &argc, argv );

        std::vector< std::function< protocol_test_fixture*() > > tests = {
            make_valid_test_case< FOO >( 6663434u ),
            make_valid_test_case< WOO >( 6663434u ),
            make_valid_test_case< TOO >( 42u ),
            make_valid_test_case< SOO >( bounded< uint8_t, 2, 4 >::get< 3 >() ),
            make_valid_test_case< KOO >( 42u ) };

        exec_protocol_test_fixture_test( tests );
        return RUN_ALL_TESTS();
}
