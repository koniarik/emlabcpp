/// MIT License
///
/// Copyright (c) 2025-2026 Jan Veverak Koniarik
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

#include "emlabcpp/hash.hpp"

#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <span>

namespace emlabcpp
{

TEST( Hash, fnv1a )
{
        static_assert( fnv1a( "" ) == 0x811C'9DC5U );
        EXPECT_EQ( fnv1a( "a" ), 0xE40C'292CU );
        EXPECT_EQ( fnv1a( "foobar" ), 0xBF9C'F968U );
}

TEST( Hash, fnv1a_bytes )
{
        static constexpr std::array< uint8_t, 6 > foobar{ 'f', 'o', 'o', 'b', 'a', 'r' };
        static_assert( fnv1a( std::span< uint8_t const >{} ) == fnv1a_offset_basis );
        EXPECT_EQ( fnv1a( foobar ), 0xBF9C'F968U );
        EXPECT_EQ( fnv1a( std::span{ foobar }.subspan( 3 ), fnv1a( "foo" ) ), fnv1a( "foobar" ) );
}

TEST( Hash, fnv1a_streaming )
{
        EXPECT_EQ( fnv1a( "bar", fnv1a( "foo" ) ), fnv1a( "foobar" ) );
        EXPECT_EQ( fnv1a( "", fnv1a( "foobar" ) ), fnv1a( "foobar" ) );
}

}  // namespace emlabcpp
