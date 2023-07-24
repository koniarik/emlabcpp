#include "emlabcpp/experimental/logging/color.h"
#include "emlabcpp/experimental/logging/time.h"
#include "emlabcpp/experimental/pretty_printer.h"
#include "emlabcpp/match.h"

#include <iostream>

#pragma once

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

class eabi_logger
{
public:
        explicit eabi_logger( const std::vector< logging_option >& opts )
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
                            this->write( sub );
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

}  // namespace emlabcpp

#define EMLABCPP_EABI_LOG_IMPL( logger, file, line, ... )                  \
        do {                                                               \
                ( logger ).log_header(                                     \
                    emlabcpp::timelog( std::chrono::system_clock::now() ), \
                    emlabcpp::stem_of( ( file ) ),                         \
                    ( line ) );                                            \
                ( logger ).log( __VA_ARGS__, '\n' );                       \
        } while ( false )
