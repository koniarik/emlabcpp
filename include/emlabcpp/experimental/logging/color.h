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
