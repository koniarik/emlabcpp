#include "either.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

TEST(Either, assemble_left_collect_right) {
        using error         = int;
        using test_either   = either<std::string, error>;
        using test_either_2 = either<float, error>;

        either<std::tuple<std::string, std::string, float>, static_circular_buffer<error, 3>>
            assemble_either = assemble_left_collect_right(test_either{"wolololo"},
                                                          test_either{"nope"}, test_either_2{0.f});

        EXPECT_TRUE(assemble_either.is_left());

        auto assemble_either_2 =
            assemble_left_collect_right(test_either{"wololo"}, test_either{666});

        EXPECT_FALSE(assemble_either_2.is_left());
}
