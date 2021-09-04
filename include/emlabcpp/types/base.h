#include <array>
#include <tuple>
#include <type_traits>
#include <vector>

#pragma once

namespace emlabcpp
{
// ------------------------------------------------------------------------------------------------
/// iterator_of is structure where iterator_of<Container>::type returns type of iterator that is
/// returned by cont.begin();
template < typename Container >
struct iterator_of
{
        using type =
            decltype( std::begin( std::declval< std::remove_reference_t< Container > >() ) );
};

template < typename Container >
using iterator_of_t = typename iterator_of< Container >::type;

// ------------------------------------------------------------------------------------------------
/// is_view<T>::value marks whenever is some type of temporary view - not owning of the data
namespace impl
{
        template < typename >
        struct is_view : std::false_type
        {
        };

        template < std::ranges::borrowed_range T >
        struct is_view< T > : std::true_type
        {
        };

}  // namespace impl

template < typename T >
struct is_view : impl::is_view< std::decay_t< T > >
{
};

template < typename T >
constexpr bool is_view_v = is_view< T >::value;

// ------------------------------------------------------------------------------------------------
/// are_same<Ts..>::value is true if all Ts... are equal types.
template < typename... >
struct are_same;

template < typename T, typename... Ts >
struct are_same< T, Ts... > : std::conjunction< std::is_same< T, Ts >... >
{
};

template <>
struct are_same<> : std::true_type
{
};

template < typename... Ts >
constexpr bool are_same_v = are_same< Ts... >::value;

// ------------------------------------------------------------------------------------------------
/// tuple_has_type<T, Tuple>::value is true if Tuple s std::tuple and contains type T
template < typename T, typename Tuple >
struct tuple_has_type;

template < typename T, typename... Us >
struct tuple_has_type< T, std::tuple< Us... > > : std::disjunction< std::is_same< T, Us >... >
{
};

template < typename T, typename... Us >
constexpr bool tuple_has_type_v = tuple_has_type< T, Us... >::value;

// ------------------------------------------------------------------------------------------------
/// is_std_tuple<T>::value is true if type T is std::tuple
namespace impl
{
        template < typename >
        struct is_std_tuple : std::false_type
        {
        };

        template < typename... T >
        struct is_std_tuple< std::tuple< T... > > : std::true_type
        {
        };
}  // namespace impl

template < typename T >
struct is_std_tuple : impl::is_std_tuple< std::decay_t< T > >
{
};

template < typename T >
constexpr bool is_std_tuple_v = is_std_tuple< T >::value;

// ------------------------------------------------------------------------------------------------
/// is_std_array<T>::value is true if type T is std::array
namespace impl
{
        template < typename >
        struct is_std_array : std::false_type
        {
        };

        template < typename T, std::size_t N >
        struct is_std_array< std::array< T, N > > : std::true_type
        {
        };
}  // namespace impl

template < typename T >
struct is_std_array : impl::is_std_array< std::decay_t< T > >
{
};

template < typename T >
constexpr bool is_std_array_v = is_std_array< T >::value;

// ------------------------------------------------------------------------------------------------
/// is_std_vector<T>::value is true if type T is std::vector
namespace impl
{
        template < typename >
        struct is_std_vector : std::false_type
        {
        };

        template < typename T >
        struct is_std_vector< std::vector< T > > : std::true_type
        {
        };
}  // namespace impl

template < typename T >
struct is_std_vector : impl::is_std_vector< std::decay_t< T > >
{
};

template < typename T >
constexpr bool is_std_vector_v = is_std_vector< T >::value;

// ------------------------------------------------------------------------------------------------
/// static_size<T>::value is size of the type T, if it has any deducable at compile time

namespace impl
{
        template < typename >
        struct static_size;

        template < typename T, std::size_t N >
        struct static_size< std::array< T, N > >
        {
                static constexpr std::size_t value = N;
        };

        template < typename... Ts >
        struct static_size< std::tuple< Ts... > >
        {
                static constexpr std::size_t value = std::tuple_size_v< std::tuple< Ts... > >;
        };
}  // namespace impl

template < typename T >
struct static_size : impl::static_size< std::decay_t< T > >
{
};

/// Marked deprecated on 19.4.2021
template < typename T >
[[deprecated]] constexpr std::size_t static_size_v = static_size< T >::value;

}  // namespace emlabcpp
