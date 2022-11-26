#include "emlabcpp/experimental/logging/severity.h"
#include "emlabcpp/concepts.h"

#include <string_view>

#pragma once

namespace emlabcpp
{
struct log_colors
{
        std::string_view debug;
        std::string_view info;
};

static constexpr log_colors time_colors = { .debug = "250", .info = "33" };
static constexpr log_colors file_colors = { .debug = "252", .info = "128" };
static constexpr log_colors line_colors = { .debug = "248", .info = "164" };

struct log_color_stub
{
        std::string_view c;
};

auto& operator<<( ostreamlike auto& os, const log_color_stub& c )
{
        return os << "\033[38;5;" << c.c << "m";
}

consteval std::string_view select_color( const log_colors& lc, log_severity sever )
{
        switch ( sever ) {
                case log_severity::DEBUG:
                        return lc.debug;
                case log_severity::INFO:
                        return lc.info;
        }
        return "";  // TODO: might rethink this
}

consteval log_color_stub log_color( const log_colors& lc, log_severity sever )
{
        return log_color_stub{ select_color( lc, sever ) };
}

consteval std::string_view resetcolor()
{
        return "\033[0m";
}
}  // namespace emlabcpp
