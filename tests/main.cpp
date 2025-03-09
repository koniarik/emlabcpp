
#include "util/util.h"

#include <gtest/gtest.h>

int main( int argc, char** argv )
{
        testing::InitGoogleTest( &argc, argv );

        using namespace emlabcpp;

        protocol_def_tests();
        protocol_sophisticated_tests();
        protocol_register_map_tests();

        return RUN_ALL_TESTS();
}
