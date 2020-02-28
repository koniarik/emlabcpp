#inlude "view.h"
#include <tuple>

#pragma once

namespace emlabcpp {

/* zip_ierator iterates over a group of iterators, where value is a tuple of references to value for
 * each iterator.
 *
 * The design expects that all ranges of iterators are of same size.
 */
template <typename... Iterators>
class zip_iterator {
        std::tuple<Iterators...> iters_;

      public:
        constexpr zip_iterator(Iterators... iters) : iters_(std::move(iters)...) {}

        // Increases each iterator
        constexpr zip_iterator operator++() {
                std::apply(
                    [](auto &&... it) { //
                            (++it, ...);
                    },
                    iters_);

                return *this;
        }

        // Decreases each iterator
        constexpr zip_iterator operator--() {
                std::apply(
                    [](auto &&... it) { //
                            (++it, ...);
                    },
                    iters_);

                return *this;
        }

        // Dereference of each iterator, returns tuple of references to the
        // operator* of iterators.
        constexpr auto operator*() {
                return std::apply(
                    [](auto &&... it) { //
                            return std::forward_as_tuple((*it)...);
                    },
                    iters_);
        }

        // Two zip iterators are equal if all of their iterators are equal
        constexpr bool operator==(const zip_iterator<Iterators...> &other) const {
                return equals(other, std::index_sequence_for<Iterators...>{});
        }

      private:
        template <typename std::size_t... Idx>
        constexpr bool equals(const zip_iterator<Iterators...> &other,
                              std::index_sequence<Idx...>) const {
                return ((std::get<Idx>(iters_) == std::get<Idx>(other.iters_)) || ...);
        }
};

template <typename... Iterators>
constexpr bool operator!=(const zip_iterator<Iterators...> &lh,
                          const zip_iterator<Iterators...> &rh) {
        return !(lh == rh);
}

/* Creates a view of zip iterators for specified containers.
 *
 * Beware that the function does not check that containers have same size of
 * ranges. If the size differs, increments of begin iterator will never be same
 * as end iterator.
 */
template <typename... Ts>
inline auto zip(Ts &&... cont) {
        return view(zip_iterator(std::begin(cont)...), zip_iterator(std::end(cont)...));
}

} // namespace emlabcpp
