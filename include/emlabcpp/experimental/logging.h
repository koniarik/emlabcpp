#include <string_view>

#pragma once

namespace emlabcpp
{
consteval std::string_view stem_of( const char* const file )
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
#include "emlabcpp/experimental/pretty_printer.h"
#include "emlabcpp/experimental/simple_stream.h"

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
        using ostream = pretty_printer< simple_stream< gpos_logger& > >;

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

        ostream& time_stream()
        {
                set_color( colors_.time );
                return os_;
        }
        ostream& file_stream()
        {
                set_color( colors_.file );
                return os_;
        }
        ostream& line_stream()
        {
                set_color( colors_.line );
                return os_;
        }
        ostream& msg_stream()
        {
                write_std_streams( reset_color() );
                return os_;
        }

        void operator()( const std::string_view sv )
        {
                write_std_streams( sv );
                if ( filestream_ ) {
                        filestream_->write(
                            sv.data(), static_cast< std::streamsize >( sv.size() ) );
                }
        }

private:
        void set_color( const std::string_view c )
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

        ostream os_{ simple_stream< gpos_logger& >{ *this } };

        log_colors colors_{};

        bool          use_stdout_ = false;
        bool          use_stderr_ = false;
        std::ostream* filestream_;
};

extern gpos_logger INFO_LOGGER;
extern gpos_logger DEBUG_LOGGER;

}  // namespace emlabcpp

#define EMLABCPP_LOG_IMPL( msg, logger )                                             \
        do {                                                                         \
                ( logger ).time_stream()                                             \
                    << emlabcpp::timelog( std::chrono::system_clock::now() ) << " "; \
                ( logger ).file_stream() << emlabcpp::stem_of( __FILE__ ) << ":";    \
                ( logger ).line_stream() << __LINE__ << " ";                         \
                ( logger ).msg_stream() << msg << "\n";                              \
        } while ( false )

#define EMLABCPP_LOG( msg ) EMLABCPP_LOG_IMPL( msg, emlabcpp::INFO_LOGGER )
#define EMLABCPP_DEBUG_LOG( msg ) EMLABCPP_LOG_IMPL( msg, emlabcpp::DEBUG_LOGGER )

#elif defined EMLABCPP_USE_NONEABI_LOGGING

#include "emlabcpp/experimental/pretty_printer.h"
#include "emlabcpp/experimental/simple_stream.h"

namespace emlabcpp
{

struct logger
{
        void start();
        void write( std::string_view );
        void end();
};

extern logger LOGGER;

void log_to_global_logger( std::string_view sv );

}  // namespace emlabcpp

#define EMLABCPP_LOG_IMPL( msg )                                                 \
        do {                                                                     \
                emlabcpp::LOGGER.start();                                        \
                emlabcpp::pretty_printer pp{                                     \
                    emlabcpp::simple_stream{ emlabcpp::log_to_global_logger } }; \
                pp << msg;                                                       \
                emlabcpp::LOGGER.end();                                          \
        } while ( false )

#define EMLABCPP_LOG( msg ) EMLABCPP_LOG_IMPL( msg )
#define EMLABCPP_DEBUG_LOG( msg ) EMLABCPP_LOG_IMPL( msg )

#else

#define EMLABCPP_LOG( msg )
#define EMLABCPP_DEBUG_LOG( msg )

#endif
