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

#include "emlabcpp/concepts.h"

#include <string_view>

#pragma once

namespace emlabcpp
{

struct log_colors
{
        std::string_view time;
        std::string_view file;
        std::string_view line;
};

static constexpr log_colors INFO_LOGGER_COLORS  = { .time = "33", .file = "128", .line = "164" };
static constexpr log_colors DEBUG_LOGGER_COLORS = { .time = "250", .file = "252", .line = "248" };
static constexpr log_colors ERROR_LOGGER_COLORS = { .time = "196", .file = "197", .line = "198" };

struct log_color_stub
{
        std::string_view c;
};

auto& operator<<( ostreamlike auto& os, const log_color_stub& c )
{
        return os << "\033[38;5;" << c.c << "m";
}

consteval std::string_view reset_color()
{
        return "\033[0m";
}
}  // namespace emlabcpp
