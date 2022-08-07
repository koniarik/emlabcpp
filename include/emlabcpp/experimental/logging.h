#pragma once

#ifdef EMLABCPP_LOGGING_ENABLED
#include "emlabcpp/experimental/pretty_printer.h"

#include <filesystem>
#include <iostream>

namespace emlabcpp
{
inline void log( const char* file, int line, const std::string& msg )
{
        // \x1B[31mTexting\033[0m
        std::filesystem::path p{ file };
        std::time_t t = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );
        std::cout << "\033[38;5;33m" << std::put_time( std::localtime( &t ), "%T " )
                  << "\033[38;5;128m" << p.stem().native() << " \033[38;5;164m" << line
                  << "\033[0m  " << msg << "\n";
}
}  // namespace emlabcpp

#define EMLABCPP_LOG( msg ) \
        emlabcpp::log( __FILE__, __LINE__, ( emlabcpp::pretty_printer{} << msg ).str() );

#else

#define EMLABCPP_LOG( msg )

#endif

