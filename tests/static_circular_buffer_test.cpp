#include "static_circular_buffer.h"
#include <gtest/gtest.h>

using namespace emlabcpp;

TEST(static_circular_buffer_test, index_type) {
	// Tests that index type is selected correctly
	EXPECT_EQ(typeid(static_circular_buffer<int, 10>::index_type),
		  typeid(uint8_t));
	EXPECT_EQ(typeid(static_circular_buffer<int, 3000>::index_type),
		  typeid(uint16_t));
	EXPECT_EQ(typeid(static_circular_buffer<int, 111222333>::index_type),
		  typeid(uint32_t));
}

static constexpr std::size_t buffer_size = 5;

using trivial_buffer = static_circular_buffer<int, buffer_size>;
using obj_buffer = static_circular_buffer<std::string, buffer_size>;

TEST(static_circular_buffer_test, push_front) {
	trivial_buffer tbuff;
	obj_buffer obuff;

	// insert into empty buffers

	tbuff.push_front(42);
	EXPECT_EQ(tbuff.front(), 42);

	obuff.push_front("The five boxing wizards jump quickly.");
	EXPECT_EQ(obuff.front(), "The five boxing wizards jump quickly.");

	// inser into non-empty buffers

	tbuff.push_front(-1);
	EXPECT_EQ(tbuff.front(), -1);

	obuff.push_front("Nope.");
	EXPECT_EQ(obuff.front(), "Nope.");
}

TEST(static_circular_buffer_test, emplace_front) {
	trivial_buffer tbuff;
	obj_buffer obuff;

	tbuff.emplace_front(42);
	EXPECT_EQ(tbuff.front(), 42);

	// This is special constructor of std::string
	obuff.emplace_front(5, 'c');
	EXPECT_EQ(obuff.front(), "ccccc");
}

TEST(static_circular_buffer_test, pop_front) {
	trivial_buffer tbuff;
	obj_buffer obuff;

	tbuff.push_front(33);
	tbuff.push_front(42);

	EXPECT_EQ(tbuff.front(), 42);
	tbuff.pop_front();
	EXPECT_EQ(tbuff.front(), 33);

	obuff.push_front("Nope.");
	obuff.push_front("Yes");

	EXPECT_EQ(obuff.front(), "Yes");
	obuff.pop_front();
	EXPECT_EQ(obuff.front(), "Nope.");
}

TEST(static_circular_buffer_test, push_back) {
	trivial_buffer tbuff;
	obj_buffer obuff;

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

TEST(static_circular_buffer_test, emplace_back) {
	trivial_buffer tbuff;
	obj_buffer obuff;

	tbuff.emplace_back(42);
	EXPECT_EQ(tbuff.back(), 42);

	// This is special constructor of std::string
	obuff.emplace_back(5, 'c');
	EXPECT_EQ(obuff.back(), "ccccc");
}

TEST(static_circular_buffer_test, pop_back) {
	trivial_buffer tbuff;
	obj_buffer obuff;

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

TEST(static_circular_buffer_test, circler_overflow_trivial) {
	std::vector<int> tidata = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
	std::vector<int> todata;
	trivial_buffer tbuff;

	EXPECT_GT(tidata.size(), buffer_size);

	// insert data from tidata into tbuff, if it is full start poping the
	// data from tbuff to todata
	for (int i : tidata) {
		if (tbuff.full()) {
			todata.push_back(tbuff.front());
			tbuff.pop_front();
		}
		tbuff.push_back(i);
	}
	// pop rest of the data in tbuff into todata
	while (!tbuff.empty()) {
		todata.push_back(tbuff.front());
		tbuff.pop_front();
	}

	// tbuff should be FIFO so the input/output dat should be equal
	EXPECT_EQ(tidata, todata);
}

TEST(static_circular_buffer_test, circle_overflow_object) {
	std::vector<std::string> oidata = {"Pack", "my",    "box",    "with",
					   "five", "dozen", "liquor", "jugs."};
	std::vector<std::string> oodata;
	obj_buffer obuff;

	EXPECT_GT(oidata.size(), buffer_size);

	for (std::string s : oidata) {
		if (obuff.full()) {
			oodata.push_back(obuff.front());
			obuff.pop_front();
		}
		obuff.push_back(s);
	}

	while (!obuff.empty()) {
		oodata.push_back(obuff.front());
		obuff.pop_front();
	}

	EXPECT_EQ(oidata, oodata);
}

TEST(static_circular_buffer_test, usage) {
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
