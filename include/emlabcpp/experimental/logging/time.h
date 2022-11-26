#include <chrono>
#include <string_view>
#include <array>

#pragma once

namespace emlabcpp
{

struct timelog
{
        std::chrono::time_point< std::chrono::system_clock > tp;
};

auto& operator<<( ostreamlike auto& os, const timelog& lg )
{
        const std::time_t t   = std::chrono::system_clock::to_time_t( lg.tp );
        const auto        dur = lg.tp.time_since_epoch();
        const auto        ms  = std::chrono::duration_cast< std::chrono::milliseconds >( dur ) %
                        std::chrono::seconds{ 1 };

        std::array< char, 42 > data;

        std::size_t i = std::strftime( data.data(), data.size(), "%T.", std::localtime( &t ) );
        os << std::string_view{ data.data(), i };

        return os << ms.count();
}

}  // namespace emlabcpp
