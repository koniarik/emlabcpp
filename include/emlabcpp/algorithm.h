#include <cmath>
#include <cstdlib>
#include <tuple>

#include "emlabcpp/algorithm/impl.h"
#include "emlabcpp/types.h"
#include "emlabcpp/view.h"

#pragma once

namespace emlabcpp {

using std::abs;
using std::max;
using std::min;

constexpr float default_epsilon = 1.19e-07f;

/// Sometimes necessary to disable warnings of unused arguments
template <typename T>
constexpr void ignore(T &&) {}

template <typename T>
struct convert_to {
        template <typename U>
        constexpr T operator()(U &&src) const noexcept(noexcept(T{std::forward<U>(src)})) {
                return T{std::forward<U>(src)};
        }
};

/// returns sign of variable T: -1,0,1
template <typename T>
constexpr int sign(T &&val) {
        using value_type = std::decay_t<T>;
        if (value_type{0} > val) {
                return -1;
        }
        if (value_type{0} < val) {
                return 1;
        }
        return 0;
}

/// maps input value 'input' from input range to equivalent value in output range
template <arithmetic_base U, arithmetic_base T>
[[nodiscard]] constexpr U map_range(T input, T from_min, T from_max, U to_min, U to_max) {
        return to_min + (to_max - to_min) * (input - from_min) / (from_max - from_min);
}

/// Returns the size of the container, regardless of what it is
template <container Container>
[[nodiscard]] constexpr std::size_t cont_size(const Container &cont) noexcept {
        if constexpr (static_sized<Container>) {
                return std::tuple_size_v<Container>;
        } else {
                return cont.size();
        }
}

/// two items 'lh' and 'rh' are almost equal if their difference is smaller than
/// value 'eps'
template <typename T>
[[nodiscard]] constexpr bool almost_equal(const T &lh, const T &rh, float eps = default_epsilon) {
        return float(abs(lh - rh)) < eps;
}

/// Returns range over Container, which skips first item of container
template <referenceable_container Container, typename Iterator = iterator_of_t<Container>>
[[nodiscard]] constexpr view<Iterator> tail(Container &&cont, int step = 1) {
        using std::begin;
        using std::end;
        return view<Iterator>(begin(cont) + step, end(cont));
}

/// Returns range over Container, which skips last item of container
template <referenceable_container Container, typename Iterator = iterator_of_t<Container>>
[[nodiscard]] constexpr view<Iterator> init(Container &&cont, int step = 1) {
        using std::begin;
        using std::end;
        return view<Iterator>(begin(cont), end(cont) - step);
}

/// Returns iterator for first item, for which call to f(*iter) holds true. end()
/// iterator is returned otherwise. The end() iterator is taken once, before the
/// container is iterated.
template <range_container Container, container_invocable<Container> UnaryFunction = std::identity>
[[nodiscard]] constexpr auto find_if(Container &&cont, UnaryFunction &&f = std::identity()) {
        using std::begin;
        auto beg = begin(cont);
        auto end = cont.end();
        for (; beg != end; ++beg) {
                if (f(*beg)) {
                        return beg;
                }
        }
        return cont.end();
}

/// Returns index of an element in tuple 't', for which call to f(x) holds true,
/// otherwise returns index of 'past the end' item - size of the tuple
template <gettable_container Container, container_invocable<Container> UnaryFunction = std::identity>
requires (!range_container<Container>)
[[nodiscard]] constexpr std::size_t find_if(Container &&t, UnaryFunction &&f = std::identity()) {
        return impl::find_if_impl(
            t, std::forward<UnaryFunction>(f),
            std::make_index_sequence<std::tuple_size_v<std::decay_t<Container>>>{});
}

/// Finds first item in container 'cont' that is equal to 'item', returns
/// iterator for container and index for tuples
template <container Container, typename T>
[[nodiscard]] constexpr auto find(Container &&cont, const T &item) {
        return find_if(cont, [&](const auto &sub_item) { return sub_item == item; });
}

/// Applies unary function 'f' to each element of container 'cont'
template <gettable_container Container, container_invocable<Container> UnaryFunction>
requires (!range_container<Container>)
constexpr void for_each(Container &&cont, UnaryFunction &&f) {
        std::apply([&](auto &&...items) { (f(std::forward<decltype(items)>(items)), ...); },
                   std::forward<Container>(cont));
}

/// Applies unary function 'f' to each element of container 'cont'
template <range_container Container, container_invocable<Container> UnaryFunction>
constexpr void for_each(Container &&cont, UnaryFunction &&f) {
        for (auto &&item : std::forward<Container>(cont)) {
                f(std::forward<decltype(item)>(item));
        }
}

/// Helper structure for finding the smallest and the largest item in some
/// container, contains min/max attributes representing such elements.
template <typename T>
struct min_max {
        T min{};
        T max{};

        min_max() = default;
        min_max(T min_i, T max_i) : min(std::move(min_i)), max(std::move(max_i)) {}
};

/// Applies unary function 'f(x)' to each element of container 'cont', returns
/// the largest and the smallest return value. of 'f(x)' calls. Returns the
/// default value of the 'f(x)' return type if container is empty.
template <container Container, container_invocable<Container> UnaryFunction = std::identity,
          typename T = std::decay_t<mapped_t<Container, UnaryFunction>>>
[[nodiscard]] constexpr min_max<T> min_max_elem(const Container &cont,
                                                UnaryFunction && f = std::identity()) {
        min_max<T> res;
        res.max = std::numeric_limits<T>::lowest();
        res.min = std::numeric_limits<T>::max();

        for_each(cont, [&](const auto &item) {
                auto val = f(item);
                res.max  = max(res.max, val);
                res.min  = min(res.min, val);
        });
        return res;
}

/// Applies unary function 'f(x)' to each element of container 'cont', returns
/// the largest return value of 'f(x)' calls. Returns lowest value of the return
/// type if container is empty.
template <container Container, container_invocable<Container> UnaryFunction = std::identity,
          typename T = std::decay_t<mapped_t<Container, UnaryFunction>>>
[[nodiscard]] constexpr T max_elem(const Container &cont, UnaryFunction &&f = std::identity()) {
        auto val = std::numeric_limits<T>::lowest();
        for_each(cont, [&](const auto &item) { //
                val = max(f(item), val);
        });
        return val;
}

/// Applies unary function 'f(x) to each element of container 'cont, returns the
/// smallest return value of 'f(x)' calls. Returns maximum value of the return
/// type if container is empty.
template <container Container, container_invocable<Container> UnaryFunction = std::identity,
          typename T = std::remove_reference_t<mapped_t<Container, UnaryFunction>>>
[[nodiscard]] constexpr T min_elem(const Container &cont, UnaryFunction &&f = std::identity()) {
        auto val = std::numeric_limits<T>::max();
        for_each(cont, [&](const auto &item) { //
                val = min(f(item), val);
        });
        return val;
}

/// Applies the unary function 'f(x)' to each element of container 'cont' and
/// returns the count of items, for which f(x) returned 'true'
template <container Container, container_invocable<Container> UnaryFunction = std::identity>
[[nodiscard]] constexpr std::size_t count(const Container &cont,
                                          UnaryFunction && f = std::identity()) {
        std::size_t res = 0;
        for_each(cont, [&](const auto &item) {
                if (f(item)) {
                        res += 1;
                }
        });
        return res;
}

/// Applies f(x) to each item of container 'cont', returns the sum of all the
/// return values of each call to 'f(x)' and 'init' item
template <container Container, container_invocable<Container> UnaryFunction = std::identity,
          typename T = std::decay_t<mapped_t<Container, UnaryFunction>>>
[[nodiscard]] constexpr T sum(const Container &cont, UnaryFunction &&f = std::identity(),
                              T init = {}) {
        for_each(cont, [&](const auto &item) { //
                init += f(item);
        });
        return init;
}

/// Applies function 'f(init,x)' to each element of container 'x' and actual
/// value of 'init' in iteration, the return value is 'init' value for next round
template <container Container, typename T, typename BinaryFunction>
[[nodiscard]] constexpr T accumulate(const Container &cont, T init, BinaryFunction &&f) {
        for_each(cont, [&](const auto &item) { //
                init = f(std::move(init), item);
        });
        return init;
}

/// Applies function 'f(x)' to each element of container 'cont' and returns the
/// average value of each call
template <container Container, container_invocable<Container> UnaryFunction = std::identity,
          typename T = std::decay_t<mapped_t<Container, UnaryFunction>>>
[[nodiscard]] constexpr T avg(const Container &cont, UnaryFunction &&f = std::identity()) {
        T res{};
        for_each(cont, [&](const auto &item) { //
                res += f(item);
        });
        if constexpr (std::is_arithmetic_v<T>) {
                return res / static_cast<T>(cont_size(cont));
        } else {
                return res / cont_size(cont);
        }
}

/// Applies binary function 'f(x,y)' to each combination of items x in lh_cont
/// and y in rh_cont
template <container LhContainer, container RhContainer, typename BinaryFunction>
constexpr void for_cross_joint(LhContainer &&lh_cont, RhContainer &&rh_cont, BinaryFunction &&f) {
        for_each(lh_cont, [&](auto &lh_item) {         //
                for_each(rh_cont, [&](auto &rh_item) { //
                        f(lh_item, rh_item);
                });
        });
}

/// Returns true if call to function 'f(x)' returns true for at least one item in
/// 'cont'
template <container Container, container_invocable<Container> UnaryFunction = std::identity>
[[nodiscard]] constexpr bool any_of(const Container &cont, UnaryFunction &&f = std::identity()) {
        auto res = find_if(cont, std::forward<UnaryFunction>(f));

        if constexpr (is_std_tuple_v<Container>) {
                return res != std::tuple_size_v<Container>;
        } else {
                return res != cont.end();
        }
}

/// Returns true if call to function 'f(x)' returns false for all items in
/// 'cont'.
template <container Container, container_invocable<Container> UnaryFunction = std::identity>
[[nodiscard]] constexpr bool none_of(const Container &cont, UnaryFunction &&f = std::identity()) {
        return !any_of(cont, std::forward<UnaryFunction>(f));
}

/// Returns true if call to function 'f(x)' returns true for all items in 'cont'
template <container Container, container_invocable<Container> UnaryFunction = std::identity>
[[nodiscard]] constexpr bool all_of(const Container &cont, UnaryFunction &&f = std::identity()) {
        return !any_of(cont, [&](const auto &item) { //
                return !f(item);
        });
}

/// Returns true of containers 'lh' and 'rh' has same size and lh[i] == rh[i] for
/// all 0 <= i < size()
template <range_container LhContainer, range_container RhContainer>
[[nodiscard]] constexpr bool equal(const LhContainer &lh, const RhContainer &rh) {
        if (lh.size() != rh.size()) {
                return false;
        }
        using std::begin;
        using std::end;
        auto lbeg = begin(lh);
        auto rbeg = begin(rh);
        auto lend = end(lh);
        for (; lbeg != lend; ++lbeg, ++rbeg) {
                if (*lbeg != *rbeg) {
                        return false;
                }
        }
        return true;
}

/// Calls function f(x) for each item in container 'cont' (or tuple) and stores
/// result in 'ResultContainer', which is returned out of the function. The
/// behavior depends on what kind of 'ResultContainer' is used, rules are in this
/// order:
///  1. std::array is constructed and res[i] = f(cont[i]) is used for i = 0...N
///  2. if 'ResultContainer' has push_back(x) method, that is used to insert
///  result of calls to f(x)
///  3. insert(x) method is used to insert result of calls to f(x)
template <impl::map_f_collectable ResultContainer, container Container,
          container_invocable<Container> UnaryFunction = std::identity>
[[nodiscard]] inline ResultContainer map_f(Container &&cont, UnaryFunction &&f = std::identity()) {
        static_assert(!is_std_tuple_v<ResultContainer>,
                      "This version of map_f does not work with std::tuple as "
                      "_result_ container!");

        ResultContainer                        res;
        impl::map_f_collector<ResultContainer> collector;

        for_each(std::forward<Container>(cont), [&](auto &&item) {
                using item_t = decltype(item);

                collector.collect(res, f(std::forward<item_t>(item)));
        });
        return res;
}

/// Calls function f(cont[i]) for i = 0...N and stores the result in array of an
/// appropiate size. The functions needs size 'N' as template parameter
template <std::size_t N, container Container,
          container_invocable<Container> UnaryFunction = std::identity,
          typename T = std::decay_t<mapped_t<Container, UnaryFunction>>>
[[nodiscard]] inline std::array<T, N>
map_f_to_a(Container &&    cont,
           UnaryFunction &&f = std::identity()) requires(!static_sized<Container>) {
        return impl::map_f_to_a_impl<T, N>(std::forward<Container>(cont),
                                           std::forward<UnaryFunction>(f),
                                           std::make_index_sequence<N>());
}

/// Calls function f(cont[i]) for i = 0...N and stores the result in array of an
/// appropiate size.
template <container Container, std::size_t N = static_size_v<Container>,
          container_invocable<Container> UnaryFunction = std::identity,
          typename T = std::decay_t<mapped_t<Container, UnaryFunction>>>
[[nodiscard]] inline std::array<T, N>
map_f_to_a(Container &&cont, UnaryFunction &&f = std::identity()) requires static_sized<Container> {
        return impl::map_f_to_a_impl<T, N>(std::forward<Container>(cont),
                                           std::forward<UnaryFunction>(f),
                                           std::make_index_sequence<N>());
}

/// Returns cont[0] + val + cont[1] + val + cont[2] + ... + cont[n-1] + val +
/// cont[n];
template <range_container Container, typename T>
[[nodiscard]] constexpr T joined(const Container &cont, T &&val) {
        if (cont.empty()) {
                return T{};
        }
        using std::begin;
        T res = *begin(cont);
        for (const auto &item : tail(cont)) {
                res += val + item;
        }
        return res;
}

struct uncurry_impl {
        template <typename Callable>
        [[nodiscard]] constexpr auto operator()(Callable &&f) const {
                return (*this) | std::forward<Callable>(f);
        }
        template <typename Callable>
        [[nodiscard]] constexpr auto operator|(Callable &&f) const {
                return [ff = std::forward<Callable>(f)](auto &&i_tuple) {
                        return std::apply(ff, std::forward<decltype(i_tuple)>(i_tuple));
                };
        }
};

/// Takes Callable object 'f', which takes multiple arguments and returns
/// function that calls 'f', but accepts tuple of arguments, rather than the
/// arguments directly
///
/// Note: returned function has copy of callable object 'f'
static constexpr uncurry_impl uncurry; // NOLINT(readability-identifier-naming)

} // namespace emlabcpp
