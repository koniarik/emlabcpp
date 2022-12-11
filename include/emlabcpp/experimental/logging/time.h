#include <array>
#include <chrono>
#include <string_view>

#pragma once

namespace emlabcpp
{

struct timelog
{
        std::chrono::time_point< std::chrono::system_clock > tp;

        timelog( const std::chrono::time_point< std::chrono::system_clock > tp_ )
          : tp( tp_ )
        {
        }
        timelog() = default;
};

auto& operator<<( ostreamlike auto& os, const timelog& lg )
{
        const std::time_t t   = std::chrono::system_clock::to_time_t( lg.tp );
        const auto        dur = lg.tp.time_since_epoch();
        const auto        ms  = std::chrono::duration_cast< std::chrono::milliseconds >( dur ) %
                        std::chrono::seconds{ 1 };

        std::array< char, 42 > data;

        const std::size_t i =
            std::strftime( data.data(), data.size(), "%T.", std::localtime( &t ) );
        os << std::string_view{ data.data(), i };

        std::snprintf( data.begin(), data.size(), "%.3li", ms.count() );
        os << std::string_view{ data.data() };

        return os;
}

}  // namespace emlabcpp
