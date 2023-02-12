#include "emlabcpp/concepts.h"
#include "emlabcpp/enum.h"
#include "emlabcpp/experimental/decompose.h"
#include "emlabcpp/match.h"
#include "emlabcpp/types.h"
#include "emlabcpp/view.h"

#include <optional>
#include <span>
#include <tuple>
#include <variant>

#ifdef EMLABCPP_USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

#pragma once

namespace emlabcpp
{

template < typename T >
struct pretty_printer;

#define EMLABCPP_PRETTY_PRINT_BASIC( bsize, w, fmt, item )                                 \
        {                                                                                  \
                std::array< char, bsize > buffer{};                                        \
                int used = snprintf( buffer.data(), buffer.size(), fmt, item );            \
                if ( used > 0 ) {                                                          \
                        std::size_t size =                                                 \
                            std::min( static_cast< std::size_t >( used ), buffer.size() ); \
                        w( std::string_view( buffer.data(), size ) );                      \
                }                                                                          \
        }                                                                                  \
        while ( false )

template < std::size_t N >
struct buffer_writer
{
        void operator()( char c )
        {
                if ( size == N ) {
                        return;
                }
                buffer[size] = c;
                size += 1;
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
              recursive_writer{ [&]( std::string_view sv ) {
                      os << sv;
              } },
              item ),
          ... );
        return os;
};

template < typename W >
struct pretty_stream
{

        template < typename U >
        pretty_stream& operator<<( const U& item )
        {
                writer( item );
                return *this;
        }

        W writer;
};

template <>
struct pretty_printer< signed char >
{
        template < typename W >
        static void print( W&& w, signed char i )
        {
                EMLABCPP_PRETTY_PRINT_BASIC( 4, w, "%hhi", i );
        }
};

template <>
struct pretty_printer< short int >
{
        template < typename W >
        static void print( W&& w, short int i )
        {
                EMLABCPP_PRETTY_PRINT_BASIC( 8, w, "%hi", i );
        }
};

template <>
struct pretty_printer< int >
{
        template < typename W >
        static void print( W&& w, int i )
        {
                EMLABCPP_PRETTY_PRINT_BASIC( 16, w, "%i", i );
        }
};

template <>
struct pretty_printer< long int >
{
        template < typename W >
        static void print( W&& w, long int i )
        {
                EMLABCPP_PRETTY_PRINT_BASIC( 32, w, "%li", i );
        }
};

template <>
struct pretty_printer< unsigned char >
{
        template < typename W >
        static void print( W&& w, unsigned char u )
        {
                EMLABCPP_PRETTY_PRINT_BASIC( 8, w, "%hhu", u );
        }
};

template <>
struct pretty_printer< short unsigned >
{
        template < typename W >
        static void print( W&& w, short unsigned u )
        {
                EMLABCPP_PRETTY_PRINT_BASIC( 8, w, "%hu", u );
        }
};

template <>
struct pretty_printer< unsigned >
{
        template < typename W >
        static void print( W&& w, unsigned u )
        {
                EMLABCPP_PRETTY_PRINT_BASIC( 16, w, "%u", u );
        }
};

template <>
struct pretty_printer< long unsigned >
{
        template < typename W >
        static void print( W&& w, long unsigned u )
        {
                EMLABCPP_PRETTY_PRINT_BASIC( 32, w, "%lu", u );
        }
};

template <>
struct pretty_printer< char >
{
        template < typename W >
        static void print( W&& w, char c )
        {
                EMLABCPP_PRETTY_PRINT_BASIC( 4, w, "%c", c );
        }
};

template < std::size_t N >
struct pretty_printer< char[N] >
{
        template < typename W >
        static void print( W&& w, const char* c )
        {
                EMLABCPP_PRETTY_PRINT_BASIC( 32, w, "%s", c );
        }
};

template <>
struct pretty_printer< std::string >
{
        template < typename W >
        static void print( W&& w, const std::string& s )
        {
                w( std::string_view{ s } );
        }
};

template <>
struct pretty_printer< bool >
{
        template < typename W >
        static void print( W&& w, bool b )
        {
                w( b ? 't' : 'f' );
        }
};

template < typename T >
requires( std::is_pointer_v< T > ) struct pretty_printer< T >
{
        template < typename W >
        static void print( W&& w, T val )
        {
                EMLABCPP_PRETTY_PRINT_BASIC( 16, w, "%p", static_cast< void* >( val ) );
        }
};

template < typename T >
requires( std::is_enum_v< T > ) struct pretty_printer< T >
{
        template < typename W >
        static void print( W&& w, T val )
        {
                w( convert_enum( val ) );
        }
};

template < typename Iterator >
struct pretty_printer< view< Iterator > >
{
        template < typename W >
        static void print( W&& w, const view< Iterator >& output )
        {
                string_serialize_view( w, output );
        }
};

template < typename T, std::size_t N >
struct pretty_printer< std::span< T, N > >
{
        template < typename W >
        static void print( W&& w, const std::span< T, N >& sp )
        {
                string_serialize_view( w, view{ sp } );
        }
};

template < typename T >
struct pretty_printer< std::vector< T > >
{
        template < typename W >
        static void print( W&& w, const std::vector< T >& vec )
        {
                string_serialize_view( w, data_view( vec ) );
        }
};

template < typename... Ts >
struct pretty_printer< std::variant< Ts... > >
{
        template < typename W >
        static void print( W&& w, const std::variant< Ts... >& var )
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
        template < typename W >
        static void print( W&& w, const std::tuple< Ts... >& tpl )
        {
                if constexpr ( sizeof...( Ts ) == 0 ) {
                        w( "()" );
                        return;
                }

                char delim = '(';
                for_each( tpl, [&]( const auto& item ) {
                        w( delim );
                        w( item );
                } );
                w( ')' );
        }
};

#ifdef EMLABCPP_USE_NLOHMANN_JSON

template <>
struct pretty_printer< nlohmann::json >
{
        template < typename W >
        static void print( W&& w, const nlohmann::json& j )
        {
                std::string s = j.dump( 4 );
                w( s );
        }
};

#endif

template < decomposable T >
struct pretty_printer< T >
{
        template < typename W >
        static void print( W&& w, const T& item )
        {
                w( decompose( item ) );
        }
};

}  // namespace emlabcpp
