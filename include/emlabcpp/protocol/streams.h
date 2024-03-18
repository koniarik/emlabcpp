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

#pragma once

#include "emlabcpp/enum.h"
#include "emlabcpp/protocol/base.h"
#include "emlabcpp/protocol/register_map.h"
#include "emlabcpp/range.h"

#include <iomanip>

namespace emlabcpp
{

template < std::size_t N >
struct pretty_printer< protocol::message< N > >
{
        template < typename T >
        static void print( T&& w, protocol::message< N > const& msg )
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
                for ( std::size_t const i : range( msg.size() ) ) {
                        if ( i % 4 == 0 )
                                l = '|';
                        auto const val = std::to_integer< uint8_t >( msg[i] );
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
        static void print( Writer&& w, protocol::sizeless_message< N > const& msg )
        {
                pretty_printer< protocol::message< N > >::print( std::forward< Writer >( w ), msg );
        }
};

template <>
struct pretty_printer< protocol::mark >
{
        template < typename T >
        static void print( T&& w, protocol::mark const& m )
        {
                w( std::string_view( m.data(), m.size() ) );
        }
};

template <>
struct pretty_printer< protocol::error_record >
{
        template < typename T >
        static void print( T&& w, protocol::error_record const& rec )
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
        inline std::ostream& operator<<( std::ostream& os, message< N > const& m )
        {
                return pretty_stream_write( os, m );
        }

        inline std::ostream& operator<<( std::ostream& os, mark const& m )
        {
                return pretty_stream_write( os, m );
        }

        inline std::ostream& operator<<( std::ostream& os, error_record const& rec )
        {
                return pretty_stream_write( os, rec );
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
}  // namespace protocol
}  // namespace emlabcpp
