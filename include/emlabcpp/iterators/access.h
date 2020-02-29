#include "emlabcpp/iterator.h"

#pragma once

namespace emlabcpp {
template <typename, typename>
class access_iterator;

template <typename Iterator, typename AccessFunction>
struct generic_iterator_traits<access_iterator<Iterator, AccessFunction>> {
        using value_type      = std::remove_reference_t<decltype(
            std::declval<AccessFunction>()(*std::declval<Iterator>()))>;
        using difference_type = std::ptrdiff_t;
        using pointer         = value_type *;
        using const_pointer   = const value_type *;
        using reference       = value_type &;
        using const_reference = const value_type &;
};

// Driver for access iterator provides access to a reference of value stored in the Iterator.
// The access is provided via the AccessFunction provided to the driver.
//
// Gives you abillity to iterate over specific value of structures for example, instead of entire
// structure.
template <typename Iterator, typename AccessFunction>
class access_iterator : public generic_iterator<access_iterator<Iterator, AccessFunction>> {
        Iterator       current_;
        AccessFunction fun_;

      public:
        using value_type =
            typename generic_iterator_traits<access_iterator<Iterator, AccessFunction>>::value_type;

        constexpr access_iterator(Iterator current, AccessFunction f)
            : current_(std::move(current)), fun_(std::move(f)) {}

        constexpr value_type &      operator*() { return fun_(*current_); }
        constexpr const value_type &operator*() const { return fun_(*current_); }

        constexpr access_iterator &operator+=(std::ptrdiff_t offset) {
                std::advance(current_, offset);
                return *this;
        }
        constexpr access_iterator &operator-=(std::ptrdiff_t offset) {
                std::advance(current_, -offset);
                return *this;
        }

        constexpr bool operator<(const access_iterator &other) const {
                return current_ < other.current_;
        }
        constexpr bool operator==(const access_iterator &other) const {
                return current_ == other.current_;
        }
};

// Creates view ver container cont with AccessFunction f.
template <typename Container, typename AccessFunction>
view<access_iterator<iterator_of_t<Container>, AccessFunction>> access_view(Container &&     cont,
                                                                            AccessFunction &&f) {
        return view{access_iterator<iterator_of_t<Container>, AccessFunction>{cont.begin(), f},
                    access_iterator<iterator_of_t<Container>, AccessFunction>{cont.end(), f}};
}

} // namespace emlabcpp
