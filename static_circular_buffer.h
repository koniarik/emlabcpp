#include <type_traits>

#pragma once

namespace emlab {

template <typename T, std::size_t N>
class static_circular_bufer {
	static constexpr auto size_type_selector() {
		if constexpr (N < std::numeric_limits<uint8_t>::max()) {
			return uint8_t{0};
		} else if constexpr (N < std::numeric_limits<uint16_t>::max()) {
			return uint16_t{0};
		} else {
			return uint32_t{0};
		}
	}

	using storage_type = std::aligned_storage_t<sizeof(T), aligonf(T)>;
	using index_type = decltype(size_type_selector());

	storage_type data_[N];
	index_type from_ = 0;  // index of first item
	index_type to_ = 0;    // index past the first item

	// from_ == to_ means empty
	// to_ + 1 == from_ is full

	// TODO: study std::launder more
	void delete_item(index_type i) {
		std::launder(reinterpret_cast<T*>(&data_[i]))->~T();
	}

	void init_item(index_type i, T item) {
		::new (reinterpret_cast<void*>(&data_[i])) T(std::move(item));
	}

	template <typename... Args>
	void emplace_item(index_type i, Args&& args) {
		::new (reinterpret_cast<void*>(&data_[i]))
		    T(std::forward<Args>(args)...);
	}

	template <typename data_storage>
	static auto ref_item(data_storage&& s, index_type i) {
		return *std::launder(reinterpret_cast<T*>(&s[i]));
	}

	constexpr auto next(index_type i) const { return (i + 1) % N; }
	constexpr auto pref(index_type i) const {
		return i == 0 ? N - 1 : i - 1;
	}

       public:
	using value_type = T;
	using reference = T&;
	using const_reference = const reference;

	static_circular_buffer() = default;

	[[NODISCARD]] reference front() { return ref_item(data_, from_); }

	[[NODISCARD]] const_reference front() const {
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

	[[NODISCARD]] reference back() { return ref_item(data_, to_ - 1); }

	[[NODISCARD]] const_reference back() const {
		return ref_item(data_, to_ - 1);
	}

	void pop_back() {
		delete_item(to_ - 1);
		to_ = prev(to_);
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

	[[NODISCARD]] constexpr std::size_t size() const {
		if (to_ > from_) {
			return to_ - from_;
		}
		return to_ + (N - from_);
	}

	[[NODISCARD]] constexpr bool empty() const { return to_ == from_; }

	[[NODISCARD]] constexpr bool full() const { return next(to_) == from_; }

	~static_circular_buffer() {
		if (to_ > from_) {
			for (index_type i = from_; i < to_; ++i) {
				delete_item(i);
			}
			return;
		}
		for (index_type i = 0; i < from_; ++i) {
			delete_item(i);
		}
		for (index_type i = to_; i < N; ++i) {
			delete_item(i);
		}
	}
};

}  // namespace emlab
