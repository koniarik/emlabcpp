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
// TODO: for actual N, max size is N-1, maybe change?
// TODO: TEST IT - UNTESTED
//
template <typename T, std::size_t N>
class static_circular_buffer {
       public:
	// public types
	// --------------------------------------------------------------------------------

	static constexpr auto size_type_selector() {
		if constexpr (N < std::numeric_limits<uint8_t>::max()) {
			return uint8_t{0};
		} else if constexpr (N < std::numeric_limits<uint16_t>::max()) {
			return uint16_t{0};
		} else {
			return uint32_t{0};
		}
	}

	// type for storage of one item
	using storage_type = std::aligned_storage_t<sizeof(T), alignof(T)>;

	// indexing type, function selects smallest type out of
	// uint8_t/uint16_t/uint32_t
	using index_type = decltype(size_type_selector());

	using value_type = T;
	using size_type = std::size_t;
	using reference = T&;
	using const_reference = const reference;

       private:
	// private attributes
	// --------------------------------------------------------------------------------

	storage_type data_[N];  // storage of the entire dataset
	index_type from_ = 0;   // index of the first item
	index_type to_ = 0;     // index past the last item

	// from_ == to_ means empty
	// to_ + 1 == from_ is full -> this practically means that it is full at
	//   size() == N - 1 items, not optimal :/

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

	void init_item(index_type i, T item) {
		::new (reinterpret_cast<void*>(&data_[i])) T(std::move(item));
	}

	template <typename... Args>
	void emplace_item(index_type i, Args&&... args) {
		::new (reinterpret_cast<void*>(&data_[i]))
		    T(std::forward<Args>(args)...);
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
	constexpr auto next(index_type i) const { return (i + 1) % N; }
	constexpr auto prev(index_type i) const {
		return i == 0 ? N - 1 : i - 1;
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
		for (index_type i = from_; i < N; ++i) {
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

	void push_front(T item) {
		from_ = prev(from_);
		init_item(from_, std::move(item));
	}

	template <typename... Args>
	void emplace_front(Args&&... args) {
		from_ = prev(from_);
		emplace_item(from_, std::forward<Args>(args)...);
	}

	// methods for handling the back side of the circular buffer

	[[nodiscard]] reference back() { return ref_item(data_, to_ - 1); }

	[[nodiscard]] const_reference back() const {
		return ref_item(data_, to_ - 1);
	}

	void pop_back() {
		to_ = prev(to_);
		delete_item(to_);
	}

	void push_back(T item) {
		init_item(to_, std::move(item));
		to_ = next(to_);
	}

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
		return to_ + (N - from_);
	}

	[[nodiscard]] constexpr bool empty() const { return to_ == from_; }

	[[nodiscard]] constexpr bool full() const { return next(to_) == from_; }
};

}  // namespace emlabcpp
