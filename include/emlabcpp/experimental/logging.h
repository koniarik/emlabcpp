#include "emlabcpp/experimental/pretty_printer.h"
#include "emlabcpp/match.h"

#include <string_view>

#pragma once

namespace emlabcpp
{
consteval std::string_view stem_of( const std::string_view file )
{
        const std::string_view res{ file };
        const std::size_t      pos = res.find_last_of( '/' );
        if ( pos == std::string_view::npos ) {
                return res;
        }
        return res.substr( pos + 1 );
}

}  // namespace emlabcpp

#ifdef EMLABCPP_USE_LOGGING

#include "emlabcpp/experimental/logging/color.h"
#include "emlabcpp/experimental/logging/time.h"

#include <iostream>

namespace emlabcpp
{

struct set_stdout
{
        bool enabled;
};

struct set_stderr
{
        bool enabled;
};

struct set_ostream
{
        std::ostream* os;
};

using logging_option = std::variant< set_stdout, set_stderr, set_ostream, log_colors >;

class gpos_logger
{
public:
        explicit gpos_logger( const std::vector< logging_option >& opts )
        {
                for ( const auto& opt : opts ) {
                        set_option( opt );
                }
        }

        void set_option( const logging_option& opt )
        {
                match(
                    opt,
                    [this]( const set_stdout& s ) {
                            use_stdout_ = s.enabled;
                    },
                    [this]( const set_stderr& s ) {
                            use_stderr_ = s.enabled;
                    },
                    [this]( const set_ostream& f ) {
                            filestream_ = f.os;
                    },
                    [this]( const log_colors& c ) {
                            colors_ = c;
                    } );
        }

        void log_header( const timelog& tl, const std::string_view file, const int line )
        {
                set_color( colors_.time );
                write( tl );

                set_color( colors_.file );
                write( ' ' );
                write( file );

                set_color( colors_.line );
                write( ':' );
                write( line );
        }

        template < typename... Args >
        void log( const Args&... args )
        {
                write_std_streams( reset_color() );
                write( ' ' );
                ( write( args ), ... );
        }

private:
        template < typename T >
        void write( const T& t )
        {

                pretty_printer< T >::print(
                    recursive_writer{ [this]( const auto& sub ) {
                            write( sub );
                    } },
                    t );
        }

        void write( const std::string_view sv )
        {

                write_std_streams( sv );
                if ( filestream_ != nullptr ) {
                        filestream_->write(
                            sv.data(), static_cast< std::streamsize >( sv.size() ) );
                }
        }

        void set_color( const std::string_view c ) const
        {
                write_std_streams( "\033[38;5;" );
                write_std_streams( c );
                write_std_streams( "m" );
        }

        void write_std_streams( const std::string_view sv ) const
        {
                if ( use_stdout_ ) {
                        std::cout.write( sv.data(), static_cast< std::streamsize >( sv.size() ) );
                }
                if ( use_stderr_ ) {
                        std::cerr.write( sv.data(), static_cast< std::streamsize >( sv.size() ) );
                }
        }

        log_colors colors_{};

        bool          use_stdout_ = false;
        bool          use_stderr_ = false;
        std::ostream* filestream_;
};

extern gpos_logger DEBUG_LOGGER;
extern gpos_logger INFO_LOGGER;
extern gpos_logger ERROR_LOGGER;

}  // namespace emlabcpp

#define EMLABCPP_LOG_IMPL( logger, file, line, ... )                       \
        do {                                                               \
                ( logger ).log_header(                                     \
                    emlabcpp::timelog( std::chrono::system_clock::now() ), \
                    emlabcpp::stem_of( ( file ) ),                         \
                    ( line ) );                                            \
                ( logger ).log( __VA_ARGS__, '\n' );                       \
        } while ( false )

#define EMLABCPP_DEBUG_LOG( ... ) \
        EMLABCPP_LOG_IMPL( emlabcpp::DEBUG_LOGGER, __FILE__, __LINE__, __VA_ARGS__ )
#define EMLABCPP_INFO_LOG( ... ) \
        EMLABCPP_LOG_IMPL( emlabcpp::INFO_LOGGER, __FILE__, __LINE__, __VA_ARGS__ )
#define EMLABCPP_ERROR_LOG( ... ) \
        EMLABCPP_LOG_IMPL( emlabcpp::ERROR_LOGGER, __FILE__, __LINE__, __VA_ARGS__ )

#elif defined EMLABCPP_USE_NONEABI_LOGGING

namespace emlabcpp
{

class noneabi_logger
{
public:
        template < typename... Args >
        void log( const Args&... args )
        {
                ( write( args ), ... );
                end();
        }

        template < typename T >
        void write( const T& t )
        {
                pretty_printer< T >::print(
                    recursive_writer{ [&]( std::string_view sv ) {
                            write( sv );
                    } },
                    t );
        }

        void write( std::string_view );
        void end();
};

extern noneabi_logger DEBUG_LOGGER;
extern noneabi_logger INFO_LOGGER;
extern noneabi_logger ERROR_LOGGER;

}  // namespace emlabcpp

#define EMLABCPP_LOG_IMPL( logger, ... )             \
        do {                                         \
                ( logger ).log( __VA_ARGS__, `\n` ); \
        } while ( false )

#define EMLABCPP_DEBUG_LOG( ... ) EMLABCPP_LOG_IMPL( emlabcpp::DEBUG_LOGGER, __VA_ARGS__ )
#define EMLABCPP_INFO_LOG( ... ) EMLABCPP_LOG_IMPL( emlabcpp::INFO_LOGGER, __VA_ARGS__ )
#define EMLABCPP_ERROR_LOG( ... ) EMLABCPP_LOG_IMPL( emlabcpp::ERROR_LOGGER, __VA_ARGS__ )

#else

#define EMLABCPP_INFO_LOG( ... )
#define EMLABCPP_DEBUG_LOG( ... )
#define EMLABCPP_ERROR_LOG( ... )

#endif
