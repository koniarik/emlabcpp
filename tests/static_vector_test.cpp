#include "emlabcpp/algorithm.h"
#include "emlabcpp/static_vector.h"
#include <gtest/gtest.h>

using namespace emlabcpp;

static constexpr std::size_t buffer_size = 10;

using trivial_buffer = static_vector<int, buffer_size>;
using obj_buffer     = static_vector<std::string, buffer_size>;

TEST(static_vector_test, pop_back) {
        trivial_buffer tbuff;
        obj_buffer     obuff;

        tbuff.push_back(33);
        tbuff.push_back(42);

        EXPECT_EQ(tbuff.back(), 42);
        tbuff.pop_back();
        EXPECT_EQ(tbuff.back(), 33);

        obuff.push_back("Nope.");
        obuff.push_back("Yes");

        EXPECT_EQ(obuff.back(), "Yes");
        obuff.pop_back();
        EXPECT_EQ(obuff.back(), "Nope.");
}

TEST(static_vector_test, push_back) {
        trivial_buffer tbuff;
        obj_buffer     obuff;

        // insert into empty buffers

        tbuff.push_back(42);
        EXPECT_EQ(tbuff.back(), 42);

        obuff.push_back("The five boxing wizards jump quickly.");
        EXPECT_EQ(obuff.back(), "The five boxing wizards jump quickly.");

        // inser into non-empty buffers

        tbuff.push_back(-1);
        EXPECT_EQ(tbuff.back(), -1);

        obuff.push_back("Nope.");
        EXPECT_EQ(obuff.back(), "Nope.");
}

TEST(static_vector_test, emplace_back) {
        trivial_buffer tbuff;
        obj_buffer     obuff;

        tbuff.emplace_back(42);
        EXPECT_EQ(tbuff.back(), 42);

        // This is special constructor of std::string
        std::size_t i = 5;
        obuff.emplace_back(i, 'c');
        EXPECT_EQ(obuff.back(), "ccccc");
}

TEST(static_vector_test, usage) {
        trivial_buffer tbuff;

        EXPECT_TRUE(tbuff.empty());
        EXPECT_FALSE(tbuff.full());
        EXPECT_EQ(tbuff.size(), 0);
        EXPECT_EQ(tbuff.max_size(), buffer_size);

        tbuff.push_back(42);

        EXPECT_FALSE(tbuff.empty());
        EXPECT_FALSE(tbuff.full());
        EXPECT_EQ(tbuff.size(), 1);
}

TEST(static_vector_test, copy_trivial) {
        trivial_buffer tbuff;
        for (int i : {1, 2, 3, 4, 5}) {
                tbuff.push_back(i);
        }

        trivial_buffer cpy{tbuff};

        EXPECT_EQ(tbuff, cpy);

        tbuff.pop_back();
        cpy.pop_back();

        EXPECT_EQ(tbuff, cpy);

        tbuff.push_back(42);

        EXPECT_NE(tbuff, cpy);

        trivial_buffer cpy2;
        cpy2 = tbuff;

        EXPECT_EQ(cpy2, tbuff);
}

TEST(static_vector_test, copy_object) {
        obj_buffer obuff;
        for (std::string s : {"Pack", "my", "box", "with", "five", "dozen", "liquor", "jugs."}) {
                obuff.push_back(s);
        }

        obj_buffer cpy{obuff};
        EXPECT_EQ(obuff, cpy);

        obj_buffer cpy2;
        cpy2 = obuff;
        EXPECT_EQ(obuff, cpy2);
}

TEST(static_vector_test, move_trivial) {
        trivial_buffer tbuff;
        for (int i : {1, 2, 3, 4, 5}) {
                tbuff.push_back(i);
        }

        trivial_buffer cpy{tbuff};
        trivial_buffer moved{std::move(tbuff)};

        EXPECT_EQ(cpy, moved);

        trivial_buffer moved2;
        moved2 = std::move(moved);

        EXPECT_EQ(cpy, moved2);
}

TEST(static_vector_test, move_object) {
        obj_buffer obuff;
        for (std::string s : {"Pack", "my", "box", "with", "five", "dozen", "liquor", "jugs."}) {
                obuff.push_back(s);
        }

        obj_buffer cpy{obuff};
        obj_buffer moved{std::move(obuff)};

        EXPECT_EQ(cpy, moved);

        obj_buffer moved2;
        moved2 = std::move(moved);

        EXPECT_EQ(cpy, moved2);
}

TEST(static_vector_test, iterators) {
        trivial_buffer   tbuff;
        std::vector<int> data = {1, 2, 3, 4, 5};
        for (int i : data) {
                tbuff.push_back(i);
        }

        std::vector<int> res;
        for (int i : tbuff) {
                res.push_back(i);
        }

        bool are_equal = equal(data, res);
        EXPECT_TRUE(are_equal);
}
