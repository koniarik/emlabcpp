#include "emlabcpp/zip.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

TEST(zip, simple) {
        std::vector<int> data = {1, 2, 3, 4, 5};

        for (auto [lh, rh] : zip(data, data)) {
                EXPECT_EQ(lh, rh);
        }
}
