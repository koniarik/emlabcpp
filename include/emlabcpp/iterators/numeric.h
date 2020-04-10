#include "emlabcpp/iterator.h"

#pragma once

namespace emlabcpp {
template <typename>
class numeric_iterator;
}

template <typename T>
struct std::iterator_traits<emlabcpp::numeric_iterator<T>> {
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = T *;
        using reference         = T &;
        using iterator_category = std::random_access_iterator_tag;
};

namespace emlabcpp {

// Driver for numeric iterator - iterator over numbers (which are calculated on the fly)
//
// Value of type T is stored internally and incremented as the iterator is moved forward/backward
template <typename T>
class numeric_iterator : public generic_iterator<numeric_iterator<T>> {
        T val_;

      public:
        // Initializes iterator to value val
        constexpr numeric_iterator(T val) : val_(std::forward<T>(val)) {}

        constexpr T &      operator*() { return val_; }
        constexpr const T &operator*() const { return val_; }

        constexpr numeric_iterator &operator+=(std::ptrdiff_t offset) {
                val_ += T(offset);
                return *this;
        }
        constexpr numeric_iterator &operator-=(std::ptrdiff_t offset) {
                val_ -= T(offset);
                return *this;
        }

        constexpr bool operator<(const numeric_iterator &other) const { return val_ < other.val_; }
        constexpr bool operator==(const numeric_iterator &other) const {
                return val_ == other.val_;
        }

        constexpr std::ptrdiff_t operator-(const numeric_iterator &other) {
                return static_cast<std::ptrdiff_t>(val_) - static_cast<std::ptrdiff_t>(other.val_);
        }
};

// Builds numeric view over interval [from, to)
template <typename Numeric, typename = std::enable_if_t<std::is_arithmetic_v<Numeric>>>
constexpr view<numeric_iterator<Numeric>> range(Numeric from, Numeric to) {
        return {numeric_iterator<Numeric>{from}, numeric_iterator<Numeric>{to}};
}

// Builds numeric view over interval [0, to)
template <typename Numeric, typename = std::enable_if_t<std::is_arithmetic_v<Numeric>>>
constexpr view<numeric_iterator<Numeric>> range(Numeric to) {
        return range<Numeric>(0, to);
}

} // namespace emlabcpp
