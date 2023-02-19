#include "emlabcpp/experimental/pretty_printer.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

TEST( pretty_printer, serialize_basic )
{
        int i = 42;
        pretty_print_serialize_basic< 16 >(
            [&]( std::string_view sv ) {
                    EXPECT_EQ( sv, "42" );
            },
            i );
}

TEST( pretty_printer, recursive_writer )
{
        int i = 42;
        pretty_print_serialize_basic< 16 >(
            recursive_writer{ [&]( std::string_view sv ) {
                    EXPECT_EQ( sv, "42" );
            } },
            i );
}

TEST( pretty_printer, pretty_stream_write )
{
        std::stringstream ss;
        int               i = 42;

        pretty_stream_write( ss, i );

        EXPECT_EQ( ss.str(), "42" );
}

}  // namespace emlabcpp
