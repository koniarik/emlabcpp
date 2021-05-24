#include "emlabcpp/concepts.h"
#include "emlabcpp/types/base.h"

#pragma once

namespace emlabcpp {

// ------------------------------------------------------------------------------------------------
/// has_static_size<T>::value is true in case type T have size deduceable at compile time

template <typename T>
constexpr bool has_static_size_v = static_sized<T>;

// ------------------------------------------------------------------------------------------------
/// mapped<T,F>::type is type returned by instance of F::operator() when applied on items from
/// instance of T. It can differentiate between tuples or containers

template <typename Container, typename UnaryFunction>
struct mapped;

template <gettable_container Container, typename UnaryFunction>
requires (!range_container<Container>)
struct mapped<Container, UnaryFunction> {
        using type = decltype(std::declval<UnaryFunction>()(std::get<0>(std::declval<Container>())));
};

template <range_container Container, typename UnaryFunction>
struct mapped<Container, UnaryFunction> {
        using type =
            decltype(std::declval<UnaryFunction>()(*std::begin(std::declval<Container>())));
};

template <typename Container, typename UnaryFunction>
using mapped_t = typename mapped<Container, UnaryFunction>::type;

// ------------------------------------------------------------------------------------------------
/// tuple_of_constants<Is..> is a tuple of integral constants in ranage Is...
template <std::size_t... Is>
using tuple_of_constants_t = std::tuple<std::integral_constant<std::size_t, Is>...>;

namespace detail {
template <typename>
struct make_sequence_tuple_impl;

template <std::size_t... Is>
struct make_sequence_tuple_impl<std::index_sequence<Is...>> {
        using type = tuple_of_constants_t<Is...>;
};
} // namespace detail

template <std::size_t N>
struct make_sequence_tuple {
        using type = typename detail::make_sequence_tuple_impl<std::make_index_sequence<N>>::type;
};

template <std::size_t N>
using make_sequence_tuple_t = typename make_sequence_tuple<N>::type;

} // namespace emlabcpp
