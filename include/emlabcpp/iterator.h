#include "emlabcpp/view.h"
#include <iterator>

#pragma once

namespace emlabcpp {
/// generic_iterator is a class using CRTP to ease implementation of custom iterators.
/// User of the class is required to inheric from generic_iterator and pass the inheriting class as
/// template argument Derived. The generic_iterator expectes existence of properly setup
/// std::iterator_traits<Derived> instance, which is used to decide types for various methods.
/// Given that generic_iterator is able to provide user with methods/functions that are based on the
/// Derived methods.
///
/// For example: It is necessary for Derived class to implement only operator==, given that
/// generic_iterator is able to provied !=
///
/// The Derived class is expected to provide these methods:
///  - reference operator*();
///  - const_reference operator*() const;
///  - Deriver& operator+=(difference_type);
///  - Deriver& operator-=(difference_type);
///  - bool operator<(const Derived & other);
///  - bool operator==(const Derived & other);
///  - difference_type operator-(const Derived& other);
///
/// Give nthese methods, the generic iterator provides additional methods thanks to CRTP mechanics:
///  - pointer operator->();
///  - const_pointer opetrator->();
///  - Derived& operator++();
///  - Derived& operator++(int);
///  - Derived& operator--();
///  - Derived& operator--(int);
///  - Derived operator+(difference_type);
///  - Derived operator-(difference_type);
///
/// Additional to that, following free functions are usable:
///  - bool operator>(const generic_iterator<Derived> &, const generic_iterator<Derived>&)
///  - bool operator<=(const generic_iterator<Derived> &, const generic_iterator<Derived>&)
///  - bool operator!=(const generic_iterator<Derived> &, const generic_iterator<Derived>&)
///
///
template <typename Derived>
class generic_iterator {

        [[nodiscard]] constexpr Derived &      impl() { return static_cast<Derived &>(*this); }
        [[nodiscard]] constexpr Derived const &impl() const {
                return static_cast<Derived const &>(*this);
        }

      public:
        using value_type        = typename std::iterator_traits<Derived>::value_type;
        using reference         = typename std::iterator_traits<Derived>::reference;
        using const_reference   = const reference;
        using pointer           = typename std::iterator_traits<Derived>::pointer;
        using const_pointer     = typename std::iterator_traits<Derived>::const_pointer;
        using difference_type   = typename std::iterator_traits<Derived>::difference_type;
        using iterator_category = typename std::iterator_traits<Derived>::iterator_category;

        constexpr pointer operator->() { return &*impl(); }

        constexpr const_pointer operator->() const { return &*impl(); }

        constexpr Derived &operator++() {
                impl() += 1;
                return impl();
        }

        constexpr Derived operator++(int) {
                auto copy = impl();
                impl() += 1;
                return copy;
        }

        constexpr Derived &operator--() {
                impl() -= 1;
                return impl();
        }

        constexpr Derived operator--(int) {
                auto copy = impl();
                impl() -= 1;
                return copy;
        }

        constexpr bool operator<(const generic_iterator<Derived> &other) const {
                return impl() < other.impl();
        }

        constexpr bool operator==(const generic_iterator<Derived> &other) const {
                return impl() == other.impl();
        }

        constexpr Derived operator+(difference_type v) const {
                auto copy = impl();
                copy += v;
                return copy;
        }

        constexpr Derived operator-(difference_type v) const {
                auto copy = impl();
                copy -= v;
                return copy;
        }
};

/// A > B iff B < A
template <typename Derived>
constexpr bool operator>(const generic_iterator<Derived> &lh, const generic_iterator<Derived> &rh) {
        return rh < lh;
}

/// A <= B iff !( B > A )
template <typename Derived>
constexpr bool operator<=(const generic_iterator<Derived> &lh,
                          const generic_iterator<Derived> &rh) {
        return !(lh > rh);
}

/// A >= B iff !( B < A )
template <typename Derived>
constexpr bool operator>=(const generic_iterator<Derived> &lh,
                          const generic_iterator<Derived> &rh) {
        return !(lh < rh);
}

/// A != B iff !( A == B)
template <typename Derived>
constexpr bool operator!=(const generic_iterator<Derived> &lh,
                          const generic_iterator<Derived> &rh) {
        return !(lh == rh);
}

} // namespace emlabcpp
