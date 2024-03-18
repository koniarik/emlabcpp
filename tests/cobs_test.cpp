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

namespace std
{

template < typename T >
vector< T > operator+( vector< T > lh, vector< T > const& rh )
{
        lh.insert( lh.end(), rh.begin(), rh.end() );
        return lh;
}

// TODO: this sucks hard
ostream& operator<<( ostream& os, byte b )
{
        return os << int( b );
}

}  // namespace std

namespace emlabcpp
{

std::vector< std::byte > v( auto&&... args )
{
        return { static_cast< std::byte >( args )... };
}

struct p
{
        std::vector< std::byte > raw;
        std::vector< std::byte > encoded;
};

std::vector< p > get_data()
{
        return {
            p{
                .raw     = v( 0x00 ),
                .encoded = v( 0x01, 0x01 ),
            },
            p{
                .raw     = v( 0x00, 0x11, 0x00 ),
                .encoded = v( 0x01, 0x02, 0x11, 0x01 ),
            },
            p{
                .raw     = v( 0x11, 0x22, 0x33, 0x44 ),
                .encoded = v( 0x05, 0x11, 0x22, 0x33, 0x44 ),
            },
            p{
                .raw     = v( 0x11, 0x22, 0x00, 0x33 ),
                .encoded = v( 0x03, 0x11, 0x22, 0x02, 0x33 ),
            },
            p{
                .raw     = v( 22, 2, 8, 1 ),
                .encoded = v( 5, 22, 2, 8, 1 ),
            },
            p{
                .raw     = v( 0x1a, 0x2, 0x50, 0x0 ),
                .encoded = v( 0x04, 0x1a, 0x2, 0x50, 0x01 ),
            },
            p{
                .raw     = std::vector( 254, std::byte{ 0x42 } ),
                .encoded = v( 0xff ) + std::vector( 254, std::byte{ 0x42 } ) + v( 0x1 ),
            },
            p{
                .raw     = std::vector( 254, std::byte{ 0x42 } ) + v( 0x00 ),
                .encoded = v( 0xff ) + std::vector( 254, std::byte{ 0x42 } ) + v( 0x1, 0x01 ),
            },
            p{
                .raw = std::vector( 254, std::byte{ 0x42 } ) + v( 0x22, 0x00, 0x33 ),
                .encoded =
                    v( 0xff ) + std::vector( 254, std::byte{ 0x42 } ) + v( 0x2, 0x22, 0x02, 0x33 ),
            },
            p{
                .raw     = std::vector( 512, std::byte{ 0x42 } ),
                .encoded = v( 0xff ) + std::vector( 254, std::byte{ 0x42 } ) +  //
                           v( 0xff ) + std::vector( 254, std::byte{ 0x42 } ) +  //
                           v( 0x5 ) + std::vector( 4, std::byte{ 0x42 } ),
            },
        };
}

TEST( COBS, encode )
{
        for ( auto [raw, encod] : get_data() ) {

                std::vector< std::byte > tmp( 1024, std::byte{ 0x00 } );

                auto [res, used] = encode_cobs( raw, tmp );
                EXPECT_TRUE( res );

                bool const are_eq = used == view{ encod };
                EXPECT_TRUE( are_eq ) << "output: " << used << "\n"
                                      << "expected: " << view{ encod };
        }
}

TEST( COBS, decode )
{
        for ( auto [raw, encod] : get_data() ) {

                std::vector< std::byte > dtmp( 1024, std::byte{ 0x00 } );

                // test decode
                auto [dres, dused] = decode_cobs( encod, dtmp );
                EXPECT_TRUE( dres );

                bool const are_eq = dused == view{ raw };
                EXPECT_TRUE( are_eq ) << "inpt:     " << view{ encod } << "\n"
                                      << "output:   " << dused << "\n"
                                      << "expected: " << view{ raw };
        }
}

TEST( COBS, decode_iter )
{
        for ( auto [raw, encod] : get_data() ) {

                auto cview = cobs_decode_view( view{ encod } );

                bool const are_eq = std::equal( raw.begin(), raw.end(), cview.begin() );
                EXPECT_TRUE( are_eq ) << "output:   " << cview << "\n"
                                      << "expected: " << view{ raw };
        }
}

}  // namespace emlabcpp
