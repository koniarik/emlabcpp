#pragma once

#ifdef EMLABCPP_LOGGING_ENABLED
#include "emlabcpp/experimental/pretty_printer.h"

#include <filesystem>
#include <iostream>

namespace emlabcpp
{
inline void log( const char* file, int line, const std::string& msg )
{
        std::filesystem::path p{ file };

        std::ostream& os = std::cout;

        auto colorize = [&]( std::string_view color, const auto& item ) {
                os << "\033[38;5;" << color << "m" << item;
                return "\033[0m";
        };

        std::time_t t = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );
        os << colorize( "33", std::put_time( std::localtime( &t ), "%T" ) ) << " "  // time
           << colorize( "128", p.stem().native() ) << " "                           // filename
           << colorize( "164", line ) << " "                                        // line
           << msg << "\n";
}
}  // namespace emlabcpp

#define EMLABCPP_LOG( msg ) \
        emlabcpp::log( __FILE__, __LINE__, ( emlabcpp::pretty_printer{} << msg ).str() );

#else

#define EMLABCPP_LOG( msg )

#endif

