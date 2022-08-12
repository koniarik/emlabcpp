#pragma once

#ifdef EMLABCPP_LOGGING_ENABLED
#include "emlabcpp/experimental/pretty_printer.h"

#include <filesystem>
#include <iostream>

namespace emlabcpp
{
void log( const char* file, int line, const std::string& msg );
}  // namespace emlabcpp

#define EMLABCPP_LOG( msg ) \
        emlabcpp::log( __FILE__, __LINE__, ( emlabcpp::pretty_printer{} << msg ).str() );

#else

#define EMLABCPP_LOG( msg )

#endif

