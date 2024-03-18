#pragma once

#include "emlabcpp/experimental/logging/color.h"
#include "emlabcpp/experimental/logging/time.h"
#include "emlabcpp/experimental/pretty_printer.h"
#include "emlabcpp/match.h"

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

class eabi_logger
{
public:
        explicit eabi_logger( std::vector< logging_option > const& opts )
        {
                for ( auto const& opt : opts )
                        set_option( opt );
        }

        void set_option( logging_option const& opt )
        {
                match(
                    opt,
                    [this]( set_stdout const& s ) {
                            use_stdout_ = s.enabled;
                    },
                    [this]( set_stderr const& s ) {
                            use_stderr_ = s.enabled;
                    },
                    [this]( set_ostream const& f ) {
                            filestream_ = f.os;
                    },
                    [this]( log_colors const& c ) {
                            colors_ = c;
                    } );
        }

        void log_header( timelog const& tl, std::string_view const file, int const line )
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
        void log( Args const&... args )
        {
                write_std_streams( reset_color() );
                write( ' ' );
                ( write( args ), ... );
        }

private:
        template < typename T >
        void write( T const& t )
        {

                pretty_printer< T >::print(
                    recursive_writer{ [this]( auto const& sub ) {
                            this->write( sub );
                    } },
                    t );
        }

        void write( std::string_view const sv )
        {

                write_std_streams( sv );
                if ( filestream_ != nullptr ) {
                        filestream_->write(
                            sv.data(), static_cast< std::streamsize >( sv.size() ) );
                        filestream_->flush();
                }
        }

        void set_color( std::string_view const c ) const
        {
                write_std_streams( "\033[38;5;" );
                write_std_streams( c );
                write_std_streams( "m" );
        }

        void write_std_streams( std::string_view const sv ) const
        {
                if ( use_stdout_ )
                        std::cout.write( sv.data(), static_cast< std::streamsize >( sv.size() ) );
                if ( use_stderr_ )
                        std::cerr.write( sv.data(), static_cast< std::streamsize >( sv.size() ) );
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
