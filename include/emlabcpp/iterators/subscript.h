#include "emlabcpp/iterator.h"

#pragma once

namespace emlabcpp {
template <typename Container>
class subscript_iterator;
}

template <typename Container>
struct std::iterator_traits<emlabcpp::subscript_iterator<Container>> {
        using value_type        = typename Container::value_type;
        using difference_type   = std::make_signed_t<std::size_t>;
        using pointer           = value_type *;
        using const_pointer     = const value_type *;
        using reference         = value_type &;
        using iterator_category = std::random_access_iterator_tag;
};

namespace emlabcpp {

// Subscript iterator stores reference to given container and index of item.
// The item is referenced via the operator[] of said container.
// Keep that in mind, as it can behave counter-inuitivelly in case you modify the container while
// you iterate it
template <typename Container>
class subscript_iterator : public generic_iterator<subscript_iterator<Container>> {

        static constexpr bool is_const = std::is_const_v<Container>;

        Container & cont_;
        std::size_t i_;

      public:
        using value_type = typename Container::value_type;
        using reference  = std::conditional_t<is_const, const value_type &, value_type &>;
        using difference_type =
            typename std::iterator_traits<subscript_iterator<Container>>::difference_type;

        subscript_iterator(Container &cont, std::size_t i) : cont_(cont), i_(i) {}

        reference       operator*() { return cont_[i_]; }
        const reference operator*() const { return cont_[i_]; }

        subscript_iterator &operator+=(std::size_t i) {
                i_ += i;
                return *this;
        }
        subscript_iterator &operator-=(std::size_t i) {
                i_ -= i;
                return *this;
        }

        bool operator<(const subscript_iterator &other) const { return i_ < other.i_; }
        bool operator==(const subscript_iterator &other) const {
                return i_ == other.i_ && &cont_ == &other.cont_;
        }

        difference_type operator-(const subscript_iterator &other) {
                return static_cast<difference_type>(i_ - other.i_);
        }
};

} // namespace emlabcpp
