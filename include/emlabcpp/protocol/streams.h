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

#include "emlabcpp/enum.h"
#include "emlabcpp/iterators/numeric.h"
#include "emlabcpp/protocol/base.h"
#include "emlabcpp/protocol/register_map.h"

#include <iomanip>

#pragma once

namespace emlabcpp::protocol
{

template < ostreamlike Stream, std::size_t N >
auto& operator<<( Stream& os, const message< N >& msg )
{
        // TODO: this might benefit from some refactoring?
        static constexpr char hex_chars[16] = {
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

        char l = '|';
        for ( std::size_t i : range( msg.size() ) ) {
                if ( i % 4 == 0 ) {
                        l = '|';
                }
                uint8_t val = msg[i];
                os << l << hex_chars[val / 16] << hex_chars[val % 16];
                l = ':';
        }
        return os;
}

auto& operator<<( ostreamlike auto& os, const mark& m )
{
        for ( char c : m ) {
                os << c;
        }
        return os;
}

auto& operator<<( ostreamlike auto& os, const error_record& rec )
{
        return os << rec.error_mark << "(" << rec.offset << ")";
}

auto& operator<<( ostreamlike auto& os, const std::endian& val )
{
        switch ( val ) {
                case std::endian::big:
                        return os << "big endian";
                case std::endian::little:
                        return os << "little endian";
        }
        return os;
}

}  // namespace emlabcpp::protocol
