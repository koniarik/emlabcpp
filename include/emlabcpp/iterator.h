#include "emlabcpp/view.h"
#include <iterator>

#pragma once

namespace emlabcpp {

template <typename T>
struct generic_iterator_traits;

template <typename T>
class generic_iterator {

        [[nodiscard]] constexpr T &      impl() { return static_cast<T &>(*this); }
        [[nodiscard]] constexpr T const &impl() const { return static_cast<T const &>(*this); }

      public:
        using value_type        = typename generic_iterator_traits<T>::value_type;
        using reference         = typename generic_iterator_traits<T>::reference;
        using const_reference   = typename generic_iterator_traits<T>::const_reference;
        using pointer           = typename generic_iterator_traits<T>::pointer;
        using const_pointer     = typename generic_iterator_traits<T>::const_pointer;
        using difference_type   = typename generic_iterator_traits<T>::difference_type;
        using iterator_category = std::bidirectional_iterator_tag;

        // Dereference to the internal driver value
        constexpr pointer operator->() { return &*impl(); }

        // Const dereference to the internal driver value
        constexpr const_pointer operator->() const { return &*impl(); }

        // Advances the driver by '1' using Driver::next(1)
        constexpr T &operator++() {
                impl() += 1;
                return impl();
        }

        // Advances the driver by '1' using Driver::next(1)
        constexpr T &operator++(int) {
                auto copy = impl();
                impl() += 1;
                return copy;
        }

        // Steps back the driver by '1' using Driver::retreat(1)
        constexpr T &operator--() {
                impl() -= 1;
                return impl();
        }

        // Steps back the driver by '1' using Driver::retreat(1)
        constexpr T &operator--(int) {
                auto copy = impl();
                impl() -= 1;
                return copy;
        }

        // Compares the iterators using Driver::less_than.
        constexpr bool operator<(const generic_iterator<T> &other) const {
                return impl() < other.impl();
        }

        // Compares the iterators using Driver::equals
        constexpr bool operator==(const generic_iterator<T> &other) const {
                return impl() == other.impl();
        }

        constexpr T operator+(difference_type v) const {
                auto copy = impl();
                copy += v;
                return copy;
        }

        constexpr T operator-(difference_type v) const {
                auto copy = impl();
                copy -= v;
                return copy;
        }
};

// A > B iff B < A
template <typename T>
constexpr bool operator>(const generic_iterator<T> &lh, const generic_iterator<T> &rh) {
        return rh < lh;
}

// A <= B iff !( B > A )
template <typename T>
constexpr bool operator<=(const generic_iterator<T> &lh, const generic_iterator<T> &rh) {
        return !(lh > rh);
}

// A >= B iff !( B < A )
template <typename T>
constexpr bool operator>=(const generic_iterator<T> &lh, const generic_iterator<T> &rh) {
        return !(lh < rh);
}

// A != B iff !( A == B)
template <typename T>
constexpr bool operator!=(const generic_iterator<T> &lh, const generic_iterator<T> &rh) {
        return !(lh == rh);
}

} // namespace emlabcpp
