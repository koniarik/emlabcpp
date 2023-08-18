#pragma once

#include "emlabcpp/experimental/pretty_printer.h"

#include <string_view>

namespace emlabcpp
{

void noneabi_log_write( std::string_view );
void noneabi_log_end();

class noneabi_logger
{
public:
        template < typename... Args >
        void log( const Args&... args )
        {
                ( write( args ), ... );
                end();
        }

        template < typename T >
        void write( const T& t )
        {
                pretty_printer< T >::print(
                    recursive_writer{ [&]( std::string_view sv ) {
                            write( sv );
                    } },
                    t );
        }

        void write( std::string_view data )
        {
                noneabi_log_write( data );
        }

        void end()
        {
                noneabi_log_end();
        };
};

}  // namespace emlabcpp

#define EMLABCPP_NONEABI_LOG_IMPL( logger, ... ) ( logger ).log( __VA_ARGS__, '\n' )
