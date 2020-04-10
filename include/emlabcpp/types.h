#include <array>
#include <tuple>
#include <type_traits>

#pragma once

namespace emlabcpp {

// ------------------------------------------------------------------------------------------------
/// iterator_of is structure where iterator_of<Container>::type returns type of iterator that is
/// returned by cont.begin();
template <typename Container>
struct iterator_of {
        using type = decltype(std::declval<std::remove_reference_t<Container>>().begin());
};

template <typename Container>
using iterator_of_t = typename iterator_of<Container>::type;

// ------------------------------------------------------------------------------------------------
/// is_view<T>::value marks whenever is some type of temporary view - not owning of the data
namespace detail {
template <typename>
struct is_view_impl : std::false_type {};
} // namespace detail

template <typename T>
constexpr bool is_view_v = detail::is_view_impl<std::decay_t<T>>::value;

// ------------------------------------------------------------------------------------------------
/// are_same<Ts..>::value is true if all Ts... are equal types.
template <typename...>
struct are_same;

template <typename T, typename... Ts>
struct are_same<T, Ts...> : std::conjunction<std::is_same<T, Ts>...> {};

template <>
struct are_same<> : std::true_type {};

template <typename... Ts>
constexpr bool are_same_v = are_same<Ts...>::value;

// ------------------------------------------------------------------------------------------------
/// tuple_has_type<T, Tuple>::value is true if Tuple s std::tuple and contains type T
template <typename T, typename Tuple>
struct tuple_has_type;

template <typename T, typename... Us>
struct tuple_has_type<T, std::tuple<Us...>> : std::disjunction<std::is_same<T, Us>...> {};

template <typename T, typename... Us>
constexpr bool tuple_has_type_v = tuple_has_type<T, Us...>::value;

// ------------------------------------------------------------------------------------------------
/// is_std_tuple<T>::value is true if type T is std::tuple
template <typename>
struct is_std_tuple : std::false_type {};

template <typename... T>
struct is_std_tuple<std::tuple<T...>> : std::true_type {};

template <typename T>
constexpr bool is_std_tuple_v = is_std_tuple<std::decay_t<T>>::value;

// ------------------------------------------------------------------------------------------------
/// is_std_array<T>::value is true if type T is std::array
template <typename>
struct is_std_array : std::false_type {};

template <typename T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};

template <typename T>
constexpr bool is_std_array_v = is_std_array<std::decay_t<T>>::value;

// ------------------------------------------------------------------------------------------------
/// static_size<T>::value is size of the type T, if it has any deducable at compile time
template <typename>
struct static_size;

template <typename T, std::size_t N>
struct static_size<std::array<T, N>> {
        static constexpr std::size_t value = N;
};

template <typename... Ts>
struct static_size<std::tuple<Ts...>> {
        static constexpr std::size_t value = std::tuple_size_v<std::tuple<Ts...>>;
};

template <typename T>
constexpr std::size_t static_size_v = static_size<std::decay_t<T>>::value;

// ------------------------------------------------------------------------------------------------
/// has_static_size<T>::value is true in case type T have size deduceable at compile time
template <typename T>
struct has_static_size {
        template <typename U, typename = decltype(static_size<std::decay_t<U>>::value)>
        static std::true_type test(int);

        template <typename>
        static std::false_type test(...);

        static constexpr bool value = decltype(test<T>(0))::value;
};

template <typename T>
constexpr bool has_static_size_v = has_static_size<std::decay_t<T>>::value;

// ------------------------------------------------------------------------------------------------
/// is_container<T>::value is true in case T is datatype with begin()/end() methods
template <typename T>
struct is_container {
        template <typename U, typename = decltype(std::declval<U>().begin()),
                  typename = decltype(std::declval<U>().end())>
        static std::true_type test(int);

        template <typename>
        static std::false_type test(...);

        static constexpr bool value = decltype(test<T>(0))::value;
};

template <typename T>
constexpr bool is_container_v = is_container<T>::value;

// ------------------------------------------------------------------------------------------------
/// has_push_back<T>::value is true in case type T has T::push_back(T::value_type) method
template <typename T>
struct has_push_back {

        template <typename U, typename = decltype(std::declval<U>().push_back(
                                  std::declval<typename U::value_type>()))>
        static std::true_type test(int);

        template <typename>
        static std::false_type test(...);

        static constexpr bool value = decltype(test<T>(0))::value;
};

template <typename T>
constexpr bool has_push_back_v = has_push_back<std::decay_t<T>>::value;

// ------------------------------------------------------------------------------------------------
/// @{
/// mapped<T,F>::type is type returned by instance of F::operator() when applied on items from
/// instance of T. It can differentiate between tuples or containers

template <typename Container, typename UnaryFunction>
struct mapped {
        using type =
            decltype(std::declval<UnaryFunction>()(*std::begin(std::declval<Container>())));
};

template <typename UnaryFunction, typename T, typename... Ts>
struct mapped<std::tuple<T, Ts...>, UnaryFunction> {
        using type = decltype(std::declval<UnaryFunction>()(std::declval<T>()));
};

template <typename UnaryFunction, typename T, typename... Ts>
struct mapped<std::tuple<T, Ts...> &, UnaryFunction> {
        T item;
        using type = decltype(std::declval<UnaryFunction>()(item));
};

template <typename UnaryFunction, typename T, typename... Ts>
struct mapped<const std::tuple<T, Ts...> &, UnaryFunction> {
        using type = decltype(std::declval<UnaryFunction>()(std::declval<T>()));
};
template <typename UnaryFunction, typename T, typename... Ts>
struct mapped<std::tuple<T, Ts...> &&, UnaryFunction> {
        using type = decltype(std::declval<UnaryFunction>()(std::declval<T>()));
};
///@}

template <typename Container, typename UnaryFunction>
using mapped_t = typename mapped<Container, UnaryFunction>::type;

// ------------------------------------------------------------------------------------------------
/// tuple_of_constants<Is..> is a tuple of integral constants in ranage Is...
template <std::size_t... Is>
using tuple_of_constants = std::tuple<std::integral_constant<std::size_t, Is>...>;

} // namespace emlabcpp