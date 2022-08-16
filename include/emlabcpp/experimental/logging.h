#pragma once

#ifdef EMLABCPP_USE_LOGGING
#include "emlabcpp/experimental/pretty_printer.h"

namespace emlabcpp
{

enum class log_severity
{
        DEBUG = 1,
        INFO  = 2
};

void log( const char* file, int line, const std::string& msg, log_severity );

}  // namespace emlabcpp

#define EMLABCPP_LOG_IMPL( msg, severity ) \
        emlabcpp::log( __FILE__, __LINE__, ( emlabcpp::pretty_printer{} << msg ).str(), severity )

#define EMLABCPP_LOG( msg ) EMLABCPP_LOG_IMPL( msg, emlabcpp::log_severity::INFO )

#define EMLABCPP_DEBUG_LOG( msg ) EMLABCPP_LOG_IMPL( msg, emlabcpp::log_severity::DEBUG )

#else

#define EMLABCPP_LOG( msg )
#define EMLABCPP_DEBUG_LOG( msg )

#endif
