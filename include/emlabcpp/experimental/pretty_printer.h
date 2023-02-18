#include "emlabcpp/concepts.h"
#include "emlabcpp/enum.h"
#include "emlabcpp/experimental/decompose.h"
#include "emlabcpp/match.h"
#include "emlabcpp/types.h"
#include "emlabcpp/view.h"

#include <charconv>
#include <chrono>
#include <filesystem>
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

template < std::size_t N, typename Writer, typename T >
void pretty_print_serialize_basic( Writer&& w, const T& val )
{
        std::array< char, N > buffer{};
        auto [ptr, ec] = std::to_chars( buffer.data(), buffer.data() + buffer.size(), val );
        if ( ec == std::errc() ) {
                w( std::string_view{
                    buffer.data(), static_cast< std::size_t >( ptr - buffer.data() ) } );
        }
}

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

        void operator()( std::string_view sv )
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

template < std::size_t N, typename... Ts >
buffer_writer< N > pretty_print_buffer( const Ts&... item )
{
        buffer_writer< N > buff{};
        ( pretty_printer< Ts >::print( recursive_writer{ buff }, item ), ... );

        return buff;
}

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
                pretty_print_serialize_basic< 4 >( w, i );
        }
};

template <>
struct pretty_printer< short int >
{
        template < typename W >
        static void print( W&& w, short int i )
        {
                pretty_print_serialize_basic< 8 >( w, i );
        }
};

template <>
struct pretty_printer< int >
{
        template < typename W >
        static void print( W&& w, int i )
        {
                pretty_print_serialize_basic< 16 >( w, i );
        }
};

template <>
struct pretty_printer< long int >
{
        template < typename W >
        static void print( W&& w, long int i )
        {
                pretty_print_serialize_basic< 32 >( w, i );
        }
};

template <>
struct pretty_printer< unsigned char >
{
        template < typename W >
        static void print( W&& w, unsigned char u )
        {
                pretty_print_serialize_basic< 4 >( w, u );
        }
};

template <>
struct pretty_printer< short unsigned >
{
        template < typename W >
        static void print( W&& w, short unsigned u )
        {
                pretty_print_serialize_basic< 16 >( w, u );
        }
};

template <>
struct pretty_printer< unsigned >
{
        template < typename W >
        static void print( W&& w, unsigned u )
        {
                pretty_print_serialize_basic< 16 >( w, u );
        }
};

template <>
struct pretty_printer< long unsigned >
{
        template < typename W >
        static void print( W&& w, long unsigned u )
        {
                pretty_print_serialize_basic< 32 >( w, u );
        }
};

template <>
struct pretty_printer< float >
{
        template < typename W >
        static void print( W&& w, float f )
        {
                pretty_print_serialize_basic< 32 >( w, f );
        }
};

template <>
struct pretty_printer< char >
{
        template < typename W >
        static void print( W&& w, char c )
        {
                w( std::string_view{ &c, 1 } );
        }
};

template < std::size_t N >
struct pretty_printer< char[N] >
{
        template < typename W >
        static void print( W&& w, const char* c )
        {
                w( std::string_view{ c, N } );
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
                w( "0x" );
                w( std::bit_cast< std::uintptr_t >( val ) );
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
                string_serialize_view( w, data_view( sp ) );
        }
};

template < typename T, std::size_t N >
struct pretty_printer< std::array< T, N > >
{
        template < typename W >
        static void print( W&& w, const std::array< T, N >& arr )
        {
                string_serialize_view( w, data_view( arr ) );
        }
};

template <>
struct pretty_printer< std::filesystem::path >
{
        template < typename W >
        static void print( W&& w, const std::filesystem::path& p )
        {
                w( p.string() );
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

template < typename Rep, typename Period >
struct pretty_printer< std::chrono::duration< Rep, Period > >
{
        template < typename W >
        static void print( W&& w, const std::chrono::duration< Rep, Period >& d )
        {
                pretty_printer< Rep >::print( w, d.count() );
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
