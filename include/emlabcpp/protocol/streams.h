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

#pragma once

#include "../enum.h"
#include "../range.h"
#include "./base.h"
#include "./register_map.h"

#include <iomanip>

namespace emlabcpp::protocol
{
template < typename T >
struct msg_format
{
        T item;
};

template < typename T >
static void pretty_print_msg_format( auto&& w, msg_format< T > wrapper )
{
        // TODO: this might benefit from some refactoring?
        static constexpr char hex_chars[16] = {
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

        char        l = '|';
        std::size_t i = 0;
        for ( std::byte const b : wrapper.item ) {
                if ( i % 4 == 0 )
                        l = '|';
                auto const val = std::to_integer< uint8_t >( b );
                w( l );
                w( hex_chars[val / 16] );
                w( hex_chars[val % 16] );
                l = ':';

                i++;
        }
}

#ifdef EMLABCPP_USE_OSTREAM
template < std::size_t N >
inline std::ostream& operator<<( std::ostream& os, const message< N >& m )
{
        pretty_print_msg_format(
            [&]( auto c ) {
                    os << c;
            },
            msg_format{ m } );
        return os;
}

inline std::ostream& operator<<( std::ostream& os, mark const& m )
{
        return os << std::string_view{ m.data(), m.size() };
}

inline std::ostream& operator<<( std::ostream& os, error_record const& rec )
{
        return os << rec.error_mark << '(' << rec.offset << ')';
}

inline std::ostream& operator<<( std::ostream& os, std::endian const& val )
{
        switch ( val ) {
        case std::endian::big:
                return os << "big endian";
        case std::endian::little:
                return os << "little endian";
        }
        return os;
}
#endif
}  // namespace emlabcpp::protocol
