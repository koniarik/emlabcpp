#include "emlabcpp/pid.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

TEST(PID, base) {
        pid::config conf;
        pid         my_pid{conf};

        EXPECT_EQ(my_pid.get_output(), 0);
}

TEST(PID, simple) {
        pid::config conf;
        conf.p   = 0.2f;
        conf.i   = 0.01f;
        conf.d   = 0.01f;
        conf.min = 0.f;
        conf.max = 100.f;

        pid my_pid{conf};

        float val     = 0.f;
        float desired = 100.f;

        for (std::size_t i = 0; i < 1000; i++) {
                my_pid.update(static_cast<float>(i), val, desired);
                val = my_pid.get_output();
        }

        EXPECT_NEAR(my_pid.get_output(), desired, 0.1f);
}
