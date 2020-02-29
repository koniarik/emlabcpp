#include "emlabcpp/types.h"

#pragma once

namespace emlabcpp {

// Generic class to represent view of some container
//
template <typename Iterator>
class view {
        Iterator begin_;
        Iterator end_;

      public:
        // standard public usings for container
        using value_type = typename std::remove_reference_t<decltype(*std::declval<Iterator>())>;
        using reverse_iterator = std::reverse_iterator<Iterator>;
        using iterator         = Iterator;

        constexpr view() = default;

        // constructor from Container, uses begin/end of the container
        template <typename Container>
        constexpr view(Container &cont) : begin_(std::begin(cont)), end_(std::end(cont)) {}

        // constructor from the iterators that should internally be stored
        constexpr view(Iterator begin, Iterator end)
            : begin_(std::move(begin)), end_(std::move(end)) {}

        // Start of the dataset
        [[nodiscard]] constexpr Iterator begin() const { return begin_; }

        // Past the end iterator
        [[nodiscard]] constexpr Iterator end() const { return end_; }

        // Access to i-th element in the range, expects Iterator::operator[]
        [[nodiscard]] constexpr decltype(auto) operator[](std::size_t i) const { return begin_[i]; }

        // Returns iterator to the last element that goes in reverse
        [[nodiscard]] constexpr reverse_iterator rbegin() const { return reverse_iterator{end_}; }

        // Returns iterator to the element before first element, that can go in
        // reverse
        [[nodiscard]] constexpr reverse_iterator rend() const { return reverse_iterator{begin_}; }

        // Size of the view over dataset uses std::distance() to tell the size
        [[nodiscard]] constexpr std::size_t size() const {
                return static_cast<std::size_t>(std::distance(begin(), end()));
        }

        // View is empty if both iterators are equal
        [[nodiscard]] constexpr bool empty() const { return begin() == end(); }

        // Returns first value of the range
        [[nodiscard]] constexpr const value_type &front() const { return *begin_; }

        // Returns last value of the range
        [[nodiscard]] constexpr const value_type &back() const { return *std::prev(end_); }
};

// The container deduction guide uses iterator_of_t
template <typename Container>
view(Container &cont)->view<iterator_of_t<Container>>;

// Support for our deduction guide to types - is_view_v
template <typename Iter>
struct detail::is_view_impl<view<Iter>> : std::true_type {};

// Creates view over 'n' items of dataset starting at 'begin'
// This does not check validity of the range!
template <typename Iter, typename Count>
constexpr view<Iter> view_n(Iter begin, Count n) {
        auto end = std::next(begin, n);
        return view<Iter>{std::move(begin), end};
}

// Creates the view over over Container, where we ignore first r*size/2 items
// and last r*size/2 items. This can be used to get the dataset without
// first/last 5% for example, by using r=0.1
template <typename Container>
constexpr view<iterator_of_t<Container>> trim_view(Container &cont, float r) {
        std::size_t step = cont.size() * (1.f - r) / 2.f;
        return {cont.begin() + step, cont.end() - step};
}

// Returns view to the Container in reverse order.
template <typename Container,
          typename = std::enable_if_t<std::is_reference_v<Container> || is_view_v<Container>>>
constexpr auto reversed(Container &&container) -> view<decltype(container.rbegin())> {
        return {container.rbegin(), container.rend()};
}

} // namespace emlabcpp
