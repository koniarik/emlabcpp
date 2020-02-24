#include "types.h"
#include <tuple>

#pragma once

namespace emlabcpp {
template <typename Container, typename UnaryFunction>
constexpr std::enable_if_t<!is_std_tuple_v<Container>, void> for_each(Container &&    cont,
                                                                      UnaryFunction &&f);

template <typename Tuple, typename UnaryFunction>
constexpr std::enable_if_t<is_std_tuple_v<Tuple>, void> for_each(Tuple &&t, UnaryFunction &&f);

namespace detail {
template <typename... Args, typename UnaryFunction, std::size_t... Idx>
[[nodiscard]] constexpr std::size_t find_if_impl(const std::tuple<Args...> &t, UnaryFunction &&f,
                                                 std::index_sequence<Idx...>) {
        std::size_t res = sizeof...(Args);
        auto        ff  = [&](const auto &item, std::size_t i) {
                if (f(item)) {
                        res = i;
                        return true;
                }
                return false;
        };

        (ff(std::get<Idx>(t), Idx) || ...);

        return res;
}
template <typename Tuple, typename UnaryFunction, std::size_t... Idx>
constexpr void for_each_impl(Tuple &&t, UnaryFunction &&f, std::index_sequence<Idx...>) {
        (f(std::get<Idx>(std::forward<Tuple>(t))), ...);
}
template <typename T, std::size_t N, typename Container, typename UnaryFunction>
[[nodiscard]] inline std::array<T, N> map_f_to_a_impl(Container &&cont, UnaryFunction &&f) {
        std::array<T, N> res;
        std::size_t      i = 0;
        emlabcpp::for_each(std::forward<Container>(cont),
                           [&](auto &&item) { //
                                   res[i] = f(std::forward<decltype(item)>(item));
                                   ++i;
                           });
        return res;
}
} // namespace detail
} // namespace emlabcpp
