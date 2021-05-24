#include <concepts>

#include "emlabcpp/types/base.h"

#pragma once

namespace emlabcpp {

template <typename T>
concept arithmetic_base = requires(T a, T b) {
        { a + b }
        ->std::convertible_to<T>;
        { a - b }
        ->std::convertible_to<T>;
        { a / b }
        ->std::convertible_to<T>;
        { a *b }
        ->std::convertible_to<T>;
};

template <typename T>
concept arithmetic_assignment = requires(T a, T b) {
        {a += b};
        {a -= b};
        {a /= b};
        {a *= b};
};

template <typename T>
concept arithmetic = arithmetic_base<T> &&arithmetic_assignment<T>;

template <typename T>
concept gettable_container = requires(T a) {
        {std::tuple_size<std::decay_t<T>>::value}->std::convertible_to<std::size_t>;
};

// so, std::ranges::range is meh because it expects return of begin() being input_output_iterator,
// which has to be def.constructible
template <typename T>
concept range_container = (
                              requires(T a) { begin(a); } || requires(T a) { std::begin(a); }) &&
                          (
                              requires(T a) { end(a); } || requires(T a) { std::end(a); });

template <typename T>
concept container = range_container<T> || gettable_container<T>;

template <typename T>
concept referenceable_container = is_view<T>::value ||
                                  (range_container<T> && !std::is_rvalue_reference_v<T>);

template <typename T>
concept static_sized = requires(T a) {
        { std::tuple_size<std::decay_t<T>>::value }
        ->std::convertible_to<std::size_t>;
};

template <typename UnaryFunction, typename Container>
concept container_invocable = requires(Container cont, UnaryFunction f) {
        f(*cont.begin());
}
|| gettable_container<Container>;

} // namespace emlabcpp

