///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
/// associated documentation files (the "Software"), to deal in the Software without restriction,
/// including without limitation the rights to use, copy, modify, merge, publish, distribute,
/// sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or substantial
/// portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
/// NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
/// OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
/// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#include "emlabcpp/experimental/pretty_printer.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

TEST( pretty_printer, serialize_basic )
{
        int i = 42;
        pretty_print_serialize_basic< 16 >(
            [&]( std::string_view sv ) {
                    EXPECT_EQ( sv, "42" );
            },
            i );
}

TEST( pretty_printer, recursive_writer )
{
        int i = 42;
        pretty_print_serialize_basic< 16 >(
            recursive_writer{ [&]( std::string_view sv ) {
                    EXPECT_EQ( sv, "42" );
            } },
            i );
}

TEST( pretty_printer, pretty_stream_write )
{
        std::stringstream ss;
        int               i = 42;

        pretty_stream_write( ss, i );

        EXPECT_EQ( ss.str(), "42" );
}

}  // namespace emlabcpp
