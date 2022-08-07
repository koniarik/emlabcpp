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

namespace emlabcpp
{

template < ostreamlike Stream, std::size_t N >
inline auto& operator<<( Stream& os, const protocol_message< N >& msg )
{
        std::ios_base::fmtflags f( os.flags() );
        char                    fill_ch = os.fill();
        std::streamsize         width   = os.width();
        os.fill( '0' );
        os.width( 2 );
        os << std::hex;
        char l = '|';
        for ( std::size_t i : range( msg.size() ) ) {
                if ( i % 4 == 0 ) {
                        l = '|';
                }
                os << l << int( msg[i] );
                l = ':';
        }
        os.fill( fill_ch );
        os.flags( f );
        os.width( width );
        return os;
}

inline auto& operator<<( ostreamlike auto& os, const protocol_mark& m )
{
        for ( char c : m ) {
                os << c;
        }
        return os;
}

inline auto& operator<<( ostreamlike auto& os, const protocol_error_record& rec )
{
        return os << rec.mark << "(" << rec.offset << ")";
}

inline auto& operator<<( ostreamlike auto& os, const protocol_endianess_enum& val )
{
        switch ( val ) {
                case PROTOCOL_BIG_ENDIAN:
                        return os << "big endian";
                case PROTOCOL_LITTLE_ENDIAN:
                        return os << "little endian";
        }
        return os;
}

template < ostreamlike Stream, protocol_endianess_enum Endianess, typename... Regs >
inline auto& operator<<( Stream& os, const protocol_register_map< Endianess, Regs... >& m )
{
        using map = protocol_register_map< Endianess, Regs... >;

        auto key_to_str = []( auto key ) {
                return convert_enum( key );
        };

        std::size_t max_key_size = 0;

        for_each_index< sizeof...( Regs ) >( [&]< std::size_t i >() {
                static constexpr auto key = map::register_key( bounded_constant< i > );
                max_key_size              = key_to_str( key ).size();
        } );

        for_each_index< sizeof...( Regs ) >( [&]< std::size_t i >() {
                static constexpr auto key = map::register_key( bounded_constant< i > );
                const auto&           val = m.template get_val< key >();

                os << std::left << std::setw( static_cast< int >( max_key_size ) )
                   << key_to_str( key ) << "\t" << val << "\n";
        } );
        return os;
}

}  // namespace emlabcpp
