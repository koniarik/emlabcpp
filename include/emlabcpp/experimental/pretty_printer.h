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

#include "emlabcpp/concepts.h"
#include "emlabcpp/enum.h"
#include "emlabcpp/experimental/decompose.h"
#include "emlabcpp/match.h"
#include "emlabcpp/min_max.h"
#include "emlabcpp/range.h"
#include "emlabcpp/types.h"
#include "emlabcpp/view.h"

#include <bitset>
#include <charconv>
#include <chrono>
#include <filesystem>
#include <map>
#include <optional>
#include <set>
#include <span>
#include <tuple>
#include <unordered_map>
#include <variant>

#ifdef EMLABCPP_USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

#pragma once

namespace emlabcpp
{

template < typename T >
struct pretty_printer;

template < std::size_t N, typename Writer, typename T >
void pretty_print_serialize_basic( Writer&& w, const T& val )
{
        std::array< char, N > buffer{};
        auto [ptr, ec] = std::to_chars( buffer.data(), buffer.data() + buffer.size(), val );
        if ( ec != std::errc() ) {
                return;
        }
        w( std::string_view{ buffer.data(), static_cast< std::size_t >( ptr - buffer.data() ) } );
}

template < std::size_t N >
struct buffer_writer
{
        void operator()( const char c )
        {
                if ( size == N ) {
                        return;
                }
                buffer[size] = c;
                size += 1;
        }

        void operator()( const std::string_view sv )
        {
                for ( char c : sv ) {
                        this->operator()( c );
                }
        }

        std::string_view sv()
        {
                return { buffer.data(), size };
        }

        std::array< char, N > buffer{};
        std::size_t           size = 0;
};

template < typename Writer >
struct recursive_writer
{
        recursive_writer( Writer w )
          : w( std::move( w ) )
        {
        }

        void operator()( std::string_view sv )
        {
                w( sv );
        }

        template < typename T >
        void operator()( const T& item )
        {
                pretty_printer< T >::print( *this, item );
        }

        Writer w;
};

template < typename... Ts >
auto& pretty_stream_write( ostreamlike auto& os, const Ts&... item )
{
        ( pretty_printer< Ts >::print(
              recursive_writer{ [&os]( std::string_view sv ) {
                      os << sv;
              } },
              item ),
          ... );
        return os;
};

template < typename T >
concept pretty_printable = requires( const T& v ) {
                                   pretty_printer< T >::print(
                                       []( const std::string_view ) {
                                               // empty intentionally
                                       },
                                       v );
                           };

template < std::size_t N, typename... Ts >
buffer_writer< N > pretty_print_buffer( const Ts&... item )
{
        buffer_writer< N > buff{};
        ( pretty_printer< Ts >::print( recursive_writer{ buff }, item ), ... );

        return buff;
}

template < typename Writer >
struct pretty_stream
{

        template < typename U >
        pretty_stream& operator<<( const U& item )
        {
                writer( item );
                return *this;
        }

        Writer writer;
};

template <>
struct pretty_printer< std::byte >
{
        template < typename Writer >
        static void print( Writer&& w, const std::byte b )
        {
                // TODO: duplicates pretty_print_serialize_basic
                std::array< char, 2 > buffer{};
                const auto [ptr, ec] = std::to_chars(
                    buffer.data(), buffer.data() + buffer.size(), static_cast< uint8_t >( b ), 16 );
                if ( ec != std::errc() ) {
                        return;
                }
                w( std::string_view{
                    buffer.data(), static_cast< std::size_t >( ptr - buffer.data() ) } );
        }
};

template <>
struct pretty_printer< signed char >
{
        template < typename Writer >
        static void print( Writer&& w, signed char i )
        {
                pretty_print_serialize_basic< 4 >( std::forward< Writer >( w ), i );
        }
};

template <>
struct pretty_printer< short int >
{
        template < typename Writer >
        static void print( Writer&& w, short int i )
        {
                pretty_print_serialize_basic< 8 >( std::forward< Writer >( w ), i );
        }
};

template <>
struct pretty_printer< int >
{
        template < typename Writer >
        static void print( Writer&& w, int i )
        {
                pretty_print_serialize_basic< 16 >( std::forward< Writer >( w ), i );
        }
};

template <>
struct pretty_printer< long int >
{
        template < typename Writer >
        static void print( Writer&& w, long int i )
        {
                pretty_print_serialize_basic< 32 >( std::forward< Writer >( w ), i );
        }
};

template <>
struct pretty_printer< unsigned char >
{
        template < typename Writer >
        static void print( Writer&& w, unsigned char u )
        {
                pretty_print_serialize_basic< 4 >( std::forward< Writer >( w ), u );
        }
};

template <>
struct pretty_printer< short unsigned >
{
        template < typename Writer >
        static void print( Writer&& w, short unsigned u )
        {
                pretty_print_serialize_basic< 16 >( std::forward< Writer >( w ), u );
        }
};

template <>
struct pretty_printer< unsigned >
{
        template < typename Writer >
        static void print( Writer&& w, unsigned u )
        {
                pretty_print_serialize_basic< 16 >( std::forward< Writer >( w ), u );
        }
};

template <>
struct pretty_printer< long unsigned >
{
        template < typename Writer >
        static void print( Writer&& w, long unsigned u )
        {
                pretty_print_serialize_basic< 32 >( std::forward< Writer >( w ), u );
        }
};

template <>
struct pretty_printer< float >
{
        template < typename Writer >
        static void print( Writer&& w, float f )
        {
                pretty_print_serialize_basic< 32 >( std::forward< Writer >( w ), f );
        }
};

template <>
struct pretty_printer< char >
{
        template < typename Writer >
        static void print( Writer&& w, const char c )
        {
                w( std::string_view{ &c, 1 } );
        }
};

template < std::size_t N >
struct pretty_printer< char[N] >
{
        template < typename Writer >
        static void print( Writer&& w, const char* const c )
        {
                // TODO: is this really a good idea?
                w( std::string_view{ c, N - 1 } );
        }
};

template <>
struct pretty_printer< std::string >
{
        template < typename Writer >
        static void print( Writer&& w, const std::string& s )
        {
                w( std::string_view{ s } );
        }
};

template <>
struct pretty_printer< bool >
{
        template < typename Writer >
        static void print( Writer&& w, const bool b )
        {
                w( b ? 't' : 'f' );
        }
};

template < typename T >
requires( std::is_pointer_v< T > )
struct pretty_printer< T >
{
        template < typename Writer >
        static void print( Writer&& w, T val )
        {
                w( "0x" );
                w( std::bit_cast< std::uintptr_t >( val ) );
        }
};

template < typename T >
requires( std::is_enum_v< T > )
struct pretty_printer< T >
{
        template < typename Writer >
        static void print( Writer&& w, T val )
        {
                w( convert_enum( val ) );
        }
};

template < typename Iterator >
struct pretty_printer< view< Iterator > >
{
        template < typename Writer >
        static void print( Writer&& w, const view< Iterator >& output )
        {
                string_serialize_view( std::forward< Writer >( w ), output );
        }
};

template < typename T, std::size_t N >
struct pretty_printer< std::span< T, N > >
{
        template < typename Writer >
        static void print( Writer&& w, const std::span< T, N >& sp )
        {
                string_serialize_view( std::forward< Writer >( w ), view{ sp } );
        }
};

template < typename T, std::size_t N >
struct pretty_printer< std::array< T, N > >
{
        template < typename Writer >
        static void print( Writer&& w, const std::array< T, N >& arr )
        {
                string_serialize_view( std::forward< Writer >( w ), view{ arr } );
        }
};

template <>
struct pretty_printer< std::filesystem::path >
{
        template < typename Writer >
        static void print( Writer&& w, const std::filesystem::path& p )
        {
                w( p.string() );
        }
};

template < typename T >
struct pretty_printer< std::optional< T > >
{
        template < typename Writer >
        static void print( Writer&& w, const std::optional< T >& opt_val )
        {
                if ( opt_val.has_value() ) {
                        w( *opt_val );
                } else {
                        w( "nothing" );
                }
        }
};

template < typename T >
struct pretty_printer< std::vector< T > >
{
        template < typename Writer >
        static void print( Writer&& w, const std::vector< T >& vec )
        {
                string_serialize_view( std::forward< Writer >( w ), data_view( vec ) );
        }
};

template < typename T >
struct pretty_printer< std::set< T > >
{
        template < typename Writer >
        static void print( Writer&& w, const std::set< T >& vec )
        {
                string_serialize_view( std::forward< Writer >( w ), view( vec ) );
        }
};

template < typename K, typename T >
struct pretty_printer< std::map< K, T > >
{
        template < typename Writer >
        static void print( Writer&& w, const std::map< K, T >& m )
        {
                string_serialize_view( std::forward< Writer >( w ), view( m ) );
        }
};

template < typename K, typename T >
struct pretty_printer< std::unordered_map< K, T > >
{
        template < typename Writer >
        static void print( Writer&& w, const std::unordered_map< K, T >& m )
        {
                string_serialize_view( std::forward< Writer >( w ), view( m ) );
        }
};

template < typename... Ts >
struct pretty_printer< std::variant< Ts... > >
{
        template < typename Writer >
        static void print( Writer&& w, const std::variant< Ts... >& var )
        {
                visit(
                    [&w]( const auto& item ) {
                            w( item );
                    },
                    var );
        }
};

template < typename... Ts >
struct pretty_printer< std::tuple< Ts... > >
{
        template < typename Writer >
        static void print( Writer&& w, const std::tuple< Ts... >& tpl )
        {
                if constexpr ( sizeof...( Ts ) == 0 ) {
                        w( "()" );
                        return;
                }

                char delim = '(';
                for_each( tpl, [&]( const auto& item ) {
                        w( delim );
                        w( item );
                        delim = ',';
                } );
                w( ')' );
        }
};

template < typename LH, typename RH >
struct pretty_printer< std::pair< LH, RH > >
{
        template < typename Writer >
        static void print( Writer&& w, const std::pair< LH, RH >& p )
        {
                w( '(' );
                w( p.first );
                w( ',' );
                w( p.second );
                w( ')' );
        }
};

template < typename Rep, typename Period >
struct pretty_printer< std::chrono::duration< Rep, Period > >
{
        template < typename Writer >
        static void print( Writer&& w, const std::chrono::duration< Rep, Period >& d )
        {
                pretty_printer< Rep >::print( std::forward< Writer >( w ), d.count() );
        }
};

template < std::size_t N >
struct pretty_printer< std::bitset< N > >
{
        template < typename Writer >
        static void print( Writer&& w, const std::bitset< N >& b )
        {
                for ( const std::size_t i : range( N ) ) {
                        w( b[i] ? '1' : '0' );
                }
        }
};

template < typename T >
struct pretty_printer< min_max< T > >
{
        template < typename Writer >
        static void print( Writer&& w, const min_max< T >& mm )
        {
                string_serialize_view( std::forward< Writer >( w ), view{ mm } );
        }
};

#ifdef EMLABCPP_USE_NLOHMANN_JSON

template <>
struct pretty_printer< nlohmann::json >
{
        template < typename Writer >
        static void print( Writer&& w, const nlohmann::json& j )
        {
                std::string s = j.dump( 4 );
                w( s );
        }
};

#endif

}  // namespace emlabcpp
