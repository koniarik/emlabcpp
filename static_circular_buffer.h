#include <cstdint>
#include <limits>
#include <new>
#include <type_traits>
#include <utility>

#pragma once

namespace emlabcpp {

// Class implementing circular buffer of any type for up to N elements.
// This should work for generic type T, not just simple types.
//
// It is safe in "single consumer single producer" scenario between main loop
// and interrupts. Because of that the behavior is as follows:
//  - on insertion, item is inserted and than index is advanced
//  - on removal, item is removed and than index is advanced
//
template <typename T, std::size_t N>
class static_circular_buffer {
	// We need real_size of the buffer to be +1 bigger than number of items
	static constexpr std::size_t real_size = N + 1;

       public:
	// public types
	// --------------------------------------------------------------------------------

	// type for storage of one item
	using storage_type = std::aligned_storage_t<sizeof(T), alignof(T)>;
	using index_type = std::size_t;

	using value_type = T;
	using size_type = std::size_t;
	using reference = T&;
	using const_reference = const reference;

       private:
	// private attributes
	// --------------------------------------------------------------------------------

	storage_type data_[real_size];  // storage of the entire dataset
	index_type from_ = 0;		// index of the first item
	index_type to_ = 0;		// index past the last item

	// from_ == to_ means empty
	// to_ + 1 == from_ is full

	// private methods
	// --------------------------------------------------------------------------------
	// To understand std::launder:
	// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0532r0.pdf
	//
	// Set of [delete,init,emplace]_item methods is necessary as data_ is
	// not array of T, but array of byte-like-type that can store T -> T
	// does not have to be initialized there. We want to fully support T
	// objects - their constructors/destructors are correctly called and we
	// do not require default constructor. This implies that data_ has some
	// slots un-initialized, some are initialized and we have to handle them
	// correctly.
	//
	// All three methods are used to handle this part of the objects in this
	// scenario, that requires features of C++ we do not want to replicate
	// and it's bettter to hide them in methods.

	void delete_item(index_type i) { ref_item(data_, i).~T(); }

	template <typename... Args>
	void emplace_item(index_type i, Args&&... args) {
		void* gen_ptr = reinterpret_cast<void*>(&data_[i]);
		::new (gen_ptr) T(std::forward<Args>(args)...);
	}

	// Reference to the item in data_storage (just trick to have shared
	// const/non-const version). std::launder is necessary here per the
	// paper linked above.
	template <typename data_storage>
	static auto& ref_item(data_storage&& s, index_type i) {
		return *std::launder(reinterpret_cast<T*>(&s[i]));
	}

	// Use this only when moving the indexes in the circular buffer -
	// bullet-proof.
	constexpr auto next(index_type i) const { return (i + 1) % real_size; }
	constexpr auto prev(index_type i) const {
		return i == 0 ? real_size - 1 : i - 1;
	}

       public:
	// public types
	// --------------------------------------------------------------------------------

	// public methods
	// --------------------------------------------------------------------------------
	static_circular_buffer() = default;

	~static_circular_buffer() {
		if (to_ > from_) {
			for (index_type i = from_; i < to_; ++i) {
				delete_item(i);
			}
			return;
		}
		for (index_type i = 0; i < to_; ++i) {
			delete_item(i);
		}
		for (index_type i = from_; i < real_size; ++i) {
			delete_item(i);
		}
	}

	// methods for handling the front side of the circular buffer

	[[nodiscard]] reference front() { return ref_item(data_, from_); }

	[[nodiscard]] const_reference front() const {
		return ref_item(data_, from_);
	}

	void pop_front() {
		delete_item(from_);
		from_ = next(from_);
	}

	// methods for handling the back side of the circular buffer

	[[nodiscard]] reference back() { return ref_item(data_, to_ - 1); }

	[[nodiscard]] const_reference back() const {
		return ref_item(data_, to_ - 1);
	}

	void push_back(T item) { emplace_back(std::move(item)); }

	template <typename... Args>
	void emplace_back(Args&&... args) {
		emplace_item(to_, std::forward<Args>(args)...);
		to_ = next(to_);
	}

	// methods for information about the usage of the buffer

	[[nodiscard]] constexpr std::size_t max_size() const { return N; }

	[[nodiscard]] constexpr std::size_t size() const {
		if (to_ >= from_) {
			return to_ - from_;
		}
		return to_ + (real_size - from_);
	}

	[[nodiscard]] constexpr bool empty() const { return to_ == from_; }

	[[nodiscard]] constexpr bool full() const { return next(to_) == from_; }
};

}  // namespace emlabcpp
