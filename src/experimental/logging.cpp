#include "emlabcpp/experimental/logging.h"

#ifdef EMLABCPP_LOGGING_ENABLED

namespace emlabcpp
{
void log( const char* file, int line, const std::string& msg )
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

#endif
