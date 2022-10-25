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

#include "emlabcpp/experimental/pretty_printer.h"

#include <iostream>

namespace emlabcpp
{

enum class log_severity
{
        DEBUG = 1,
        INFO  = 2
};

struct log_colors
{
        std::string_view debug;
        std::string_view info;
};

static constexpr log_colors time_colors = { .debug = "250", .info = "33" };
static constexpr log_colors file_colors = { .debug = "252", .info = "128" };
static constexpr log_colors line_colors = { .debug = "248", .info = "164" };

struct color
{
        std::string_view c;
};

auto& operator<<( ostreamlike auto& os, const color& c )
{
        return os << "\033[38;5;" << c.c << "m";
}

consteval std::string_view select_color( const log_colors& lc, log_severity sever )
{
        switch ( sever ) {
                case log_severity::DEBUG:
                        return lc.debug;
                case log_severity::INFO:
                        return lc.info;
        }
        return "";  // TODO: might rethink this
}

consteval color log_color( const log_colors& lc, log_severity sever )
{
        return color{ select_color( lc, sever ) };
}

consteval std::string_view resetcolor()
{
        return "\033[0m";
}

struct timelog
{
        std::chrono::time_point< std::chrono::system_clock > tp;
};

auto& operator<<( ostreamlike auto& os, const timelog& lg )
{
        const std::time_t t   = std::chrono::system_clock::to_time_t( lg.tp );
        const auto        dur = lg.tp.time_since_epoch();
        const auto        ms  = std::chrono::duration_cast< std::chrono::milliseconds >( dur ) %
                        std::chrono::seconds{ 1 };

        std::array< char, 42 > data;

        std::size_t i = std::strftime( data.data(), data.size(), "%T.", std::localtime( &t ) );
        os << std::string_view{ data.data(), i };

        return os << ms.count();
}

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

}  // namespace emlabcpp

#define EMLABCPP_LOG_IMPL( msg, severity )                                   \
        {                                                                    \
                emlabcpp::LOGGER.start();                                    \
                auto f = +[]( std::string_view sv ) {                        \
                        emlabcpp::LOGGER.write( sv );                        \
                };                                                           \
                emlabcpp::pretty_printer pp{ emlabcpp::simple_stream{ f } }; \
                pp << msg;                                                   \
                emlabcpp::LOGGER.end();                                      \
        }

#else

#define EMLABCPP_LOG_IMPL( msg, severity )

#endif

#define EMLABCPP_LOG( msg ) EMLABCPP_LOG_IMPL( msg, emlabcpp::log_severity::INFO )
#define EMLABCPP_DEBUG_LOG( msg ) EMLABCPP_LOG_IMPL( msg, emlabcpp::log_severity::DEBUG )
