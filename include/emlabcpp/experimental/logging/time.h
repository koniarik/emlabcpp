#include "emlabcpp/experimental/pretty_printer.h"

#include <array>
#include <chrono>
#include <ctime>
#include <string_view>

#pragma once

namespace emlabcpp
{

struct timelog
{
        std::chrono::time_point< std::chrono::system_clock > tp;

        explicit timelog( const std::chrono::time_point< std::chrono::system_clock > tp_ )
          : tp( tp_ )
        {
        }
        timelog() = default;
};

template <>
struct pretty_printer< timelog >
{
        template < typename W >
        static void print( W&& w, const timelog& lg )
        {
                const std::time_t t   = std::chrono::system_clock::to_time_t( lg.tp );
                const auto        dur = lg.tp.time_since_epoch();
                const auto ms = std::chrono::duration_cast< std::chrono::milliseconds >( dur ) %
                                std::chrono::seconds{ 1 };

                std::array< char, 42 > data;

                std::tm           tmval;
                const std::size_t i =
                    std::strftime( data.data(), data.size(), "%T.", localtime_r( &t, &tmval ) );
                w( std::string_view{ data.data(), i } );

                std::snprintf( data.begin(), data.size(), "%.3li", ms.count() );
                w( std::string_view{ data.data() } );
        }
};

}  // namespace emlabcpp
