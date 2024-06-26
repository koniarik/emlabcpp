///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
/// and associated documentation files (the "Software"), to deal in the Software without
/// restriction, including without limitation the rights to use, copy, modify, merge, publish,
/// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or
/// substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
/// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
/// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#pragma once

#include "emlabcpp/experimental/pretty_printer.h"

#include <array>
#include <chrono>
#include <ctime>
#include <string_view>

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
                const auto us = std::chrono::duration_cast< std::chrono::microseconds >( dur ) %
                                std::chrono::seconds{ 1 };

                std::array< char, 42 > data;

                std::tm           tmval;
                const std::size_t i =
                    std::strftime( data.data(), data.size(), "%F %T.", localtime_r( &t, &tmval ) );
                w( std::string_view{ data.data(), i } );

                std::snprintf(
                    data.begin(), data.size(), "%.6i", static_cast< int >( us.count() ) );
                w( std::string_view{ data.data() } );
        }
};

}  // namespace emlabcpp
