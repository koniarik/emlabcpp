#include <string_view>

#pragma once

namespace emlabcpp
{
consteval std::string_view stem_of( const char* file )
{
        std::string_view res{ file };
        std::size_t      pos = res.find_last_of( '/' );
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

#include <iostream>

namespace emlabcpp
{


}  // namespace emlabcpp

#define EMLABCPP_LOG_IMPL( msg, severity )                                               \
        {                                                                                \
                emlabcpp::pretty_printer< std::ostream& > pp{ std::cout };               \
                pp << emlabcpp::log_color( emlabcpp::time_colors, severity )             \
                   << emlabcpp::timelog( std::chrono::system_clock::now() ) << "|"       \
                   << emlabcpp::log_color( emlabcpp::file_colors, severity )             \
                   << emlabcpp::stem_of( __FILE__ ) << "|"                               \
                   << emlabcpp::log_color( emlabcpp::line_colors, severity ) << __LINE__ \
                   << emlabcpp::resetcolor() << "|" << msg << "\n";                      \
        }

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

#define EMLABCPP_LOG_IMPL( msg, severity )                                       \
        {                                                                        \
                emlabcpp::LOGGER.start();                                        \
                emlabcpp::pretty_printer pp{                                     \
                    emlabcpp::simple_stream{ emlabcpp::log_to_global_logger } }; \
                pp << msg;                                                       \
                emlabcpp::LOGGER.end();                                          \
        }

#else

#define EMLABCPP_LOG_IMPL( msg, severity )

#endif

#define EMLABCPP_LOG( msg ) EMLABCPP_LOG_IMPL( msg, emlabcpp::log_severity::INFO )
#define EMLABCPP_DEBUG_LOG( msg ) EMLABCPP_LOG_IMPL( msg, emlabcpp::log_severity::DEBUG )
