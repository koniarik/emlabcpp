#include "emlabcpp/experimental/logging.h"

#ifdef EMLABCPP_LOGGING_ENABLED

#include <filesystem>
#include <iostream>

namespace emlabcpp
{
void log( const char* file, int line, const std::string& msg, log_severity severity )
{
        std::string_view tcolor = severity == log_severity::INFO ? "33" : "250";
        std::string_view fcolor = severity == log_severity::INFO ? "128" : "252";
        std::string_view lcolor = severity == log_severity::INFO ? "164" : "248";

        std::filesystem::path p{ file };

        std::ostream& os = std::cout;

        auto colorize = [&]( const std::string_view& color, const auto& item ) {
                os << "\033[38;5;" << color << "m" << item;
                return "\033[0m";
        };

        std::time_t t = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );
        os << colorize( tcolor, std::put_time( std::localtime( &t ), "%T" ) ) << " "  // time
           << colorize( fcolor, p.stem().native() ) << " "                            // filename
           << colorize( lcolor, line ) << " "                                         // line
           << msg << "\n";
}
}  // namespace emlabcpp

#endif
