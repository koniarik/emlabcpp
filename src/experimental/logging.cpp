#include "emlabcpp/experimental/logging.h"

#include "emlabcpp/static_circular_buffer.h"

#ifdef EMLABCPP_USE_LOGGING

#include <filesystem>
#include <iostream>

namespace emlabcpp
{

namespace
{
        constexpr std::size_t log_buffer_size = 128;
        using log_tp     = std::chrono::time_point< std::chrono::system_clock >;
        using log_tpl    = std::tuple< log_tp, const char*, int, std::string, log_severity >;
        using log_buffer = static_circular_buffer< log_tpl, log_buffer_size >;

        // NOLINTNEXTLINE
        thread_local log_buffer LOG_BUFFER;

        void log_print_msg(
            const log_tp       tp,
            const char* const  file,
            const int          line,
            const std::string& msg,
            const log_severity severity )
        {
                const std::string_view tcolor = severity == log_severity::INFO ? "33" : "250";
                const std::string_view fcolor = severity == log_severity::INFO ? "128" : "252";
                const std::string_view lcolor = severity == log_severity::INFO ? "164" : "248";

                const std::filesystem::path p{ file };

                std::ostream& os = std::cout;

                auto colorize = [&]( const std::string_view& color, const auto& item ) {
                        os << "\033[38;5;" << color << "m" << item;
                        return "\033[0m";
                };

                const auto dur = tp.time_since_epoch();
                const auto ms  = std::chrono::duration_cast< std::chrono::milliseconds >( dur ) %
                                std::chrono::seconds{ 1 };

                const std::time_t t = std::chrono::system_clock::to_time_t( tp );
                os << colorize( tcolor, std::put_time( std::localtime( &t ), "%T." ) )
                   << colorize( tcolor, ms.count() ) << " "         // time
                   << colorize( fcolor, p.stem().native() ) << " "  // filename
                   << colorize( lcolor, line ) << " "               // line
                   << msg << "\n";
        }
}  // namespace

void log( const char* file, int line, const std::string& msg, log_severity severity )
{
        log_tp tp = std::chrono::system_clock::now();
        if ( severity == log_severity::DEBUG ) {
                if ( LOG_BUFFER.full() ) {
                        LOG_BUFFER.pop_front();
                }
                LOG_BUFFER.emplace_back( tp, file, line, msg, severity );
                return;
        }

        for ( auto [dtp, dfile, dline, dmsg, dsever] : LOG_BUFFER ) {
                log_print_msg( dtp, dfile, dline, dmsg, dsever );
        }
        LOG_BUFFER.clear();
        log_print_msg( tp, file, line, msg, severity );
}
}  // namespace emlabcpp

#endif
