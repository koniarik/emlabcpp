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

#include "emlabcpp/experimental/cobs.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

std::vector< std::byte > v( auto&&... args )
{
        return { static_cast< std::byte >( args )... };
}

using p = std::pair< std::vector< std::byte >, std::vector< std::byte > >;

namespace std
{
// TODO: this sucks hard
std::ostream& operator<<( std::ostream& os, std::byte b )
{
        return os << int( b );
}
}  // namespace std

TEST( COBS, simple )
{
        for ( auto [raw, encod] :
              { p( v( 0x00 ), v( 0x01, 0x01 ) ),
                p( v( 0x00, 0x11, 0x00 ), v( 0x01, 0x02, 0x11, 0x01 ) ),
                p( v( 0x11, 0x22, 0x33, 0x44 ), v( 0x05, 0x11, 0x22, 0x33, 0x44 ) ),
                p( v( 0x11, 0x22, 0x00, 0x33 ), v( 0x03, 0x11, 0x22, 0x02, 0x33 ) ) } ) {

                std::vector< std::byte > tmp( 1024, std::byte{ 0x00 } );

                auto [res, used] = encode_cobs( data_view( raw ), data_view( tmp ) );
                EXPECT_TRUE( res );

                bool are_eq = used == data_view( encod );
                EXPECT_TRUE( are_eq ) << "outpt: " << used << "\n"
                                      << "expected: " << data_view( encod );

                std::vector< std::byte > dtmp( 1024, std::byte{ 0x00 } );

                auto [dres, dused] = decode_cobs( used, data_view( dtmp ) );
                EXPECT_TRUE( dres );

                are_eq = dused == data_view( raw );
                EXPECT_TRUE( are_eq ) << "outpt: " << dused << "\n"
                                      << "expected: " << data_view( raw );
        }
}
