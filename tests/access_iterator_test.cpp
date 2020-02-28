#include "algorithm.h"
#include "iterators/access.h"
#include "iterators/numeric.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

TEST(access_iterator, basic) {
        std::vector<int> vec = {42, 33, 66, -5, -22, 213};
        auto             r   = range(std::size_t{0}, vec.size());

        auto a_view = access_view(r, [&](std::size_t i) -> int & { return vec[i]; });

        bool are_equal = equal(vec, a_view);

        ASSERT_TRUE(are_equal);
}
