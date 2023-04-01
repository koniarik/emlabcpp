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

#include "emlabcpp/enum.h"
#include "emlabcpp/protocol/base.h"
#include "emlabcpp/protocol/register_map.h"
#include "emlabcpp/range.h"

#include <iomanip>

#pragma once

namespace emlabcpp
{

template < std::size_t N >
struct pretty_printer< protocol::message< N > >
{
        template < typename T >
        static void print( T&& w, const protocol::message< N >& msg )
        {
                // TODO: this might benefit from some refactoring?
                static constexpr char hex_chars[16] = {
                    '0',
                    '1',
                    '2',
                    '3',
                    '4',
                    '5',
                    '6',
                    '7',
                    '8',
                    '9',
                    'A',
                    'B',
                    'C',
                    'D',
                    'E',
                    'F' };

                char l = '|';
                for ( const std::size_t i : range( msg.size() ) ) {
                        if ( i % 4 == 0 ) {
                                l = '|';
                        }
                        const uint8_t val = msg[i];
                        w( l );
                        w( hex_chars[val / 16] );
                        w( hex_chars[val % 16] );
                        l = ':';
                }
        }
};

template < std::size_t N >
struct pretty_printer< protocol::sizeless_message< N > >
{
        template < typename Writer >
        static void print( Writer&& w, const protocol::sizeless_message< N >& msg )
        {
                pretty_printer< protocol::message< N > >::print( std::forward< Writer >( w ), msg );
        }
};

template <>
struct pretty_printer< protocol::mark >
{
        template < typename T >
        static void print( T&& w, const protocol::mark& m )
        {
                w( std::string_view( m.data(), m.size() ) );
        }
};

template <>
struct pretty_printer< protocol::error_record >
{
        template < typename T >
        static void print( T&& w, const protocol::error_record& rec )
        {
                w( rec.error_mark );
                w( '(' );
                w( rec.offset );
                w( ')' );
        }
};

namespace protocol
{
#ifdef EMLABCPP_USE_OSTREAM
        template < std::size_t N >
        inline std::ostream& operator<<( std::ostream& os, const message< N >& m )
        {
                return pretty_stream_write( os, m );
        }

        inline std::ostream& operator<<( std::ostream& os, const mark& m )
        {
                return pretty_stream_write( os, m );
        }

        inline std::ostream& operator<<( std::ostream& os, const error_record& rec )
        {
                return pretty_stream_write( os, rec );
        }

        inline std::ostream& operator<<( std::ostream& os, const std::endian& val )
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
}  // namespace protocol
}  // namespace emlabcpp
