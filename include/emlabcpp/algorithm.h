///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
/// and associated documentation files (the "Software"), to deal in the Software without
/// restriction, including without limitation the rights to use, copy, modify, merge, publish,
/// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or
/// substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
/// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
/// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#pragma once

#include "emlabcpp/algorithm/impl.h"
#include "emlabcpp/bounded.h"
#include "emlabcpp/min_max.h"
#include "emlabcpp/types.h"
#include "emlabcpp/view.h"

#include <cmath>
#include <cstdlib>
#include <tuple>

namespace emlabcpp
{

constexpr float default_epsilon = 1.19e-07f;

/// returns sign of variable T: -1,0,1
template < typename T >
constexpr int sign( const T& val )
{
        using value_type = std::decay_t< T >;
        if ( value_type{ 0 } > val )
                return -1;
        if ( value_type{ 0 } < val )
                return 1;
        return 0;
}

/// takes an value `val` and rounds it up to nearest multiply of `base`
template < typename T >
[[nodiscard]] constexpr T ceil_to( T val, T base )
{
        auto m = val % base;
        if ( m == 0 )
                return val;
        return val + base - m;
}

/// maps input value 'input' from input range to equivalent value in output range
template < arithmetic_operators T, arithmetic_operators U >
[[nodiscard]] constexpr U map_range( T input, T from_min, T from_max, U to_min, U to_max )
{
        return to_min + static_cast< U >(
                            ( to_max - to_min ) * static_cast< U >( input - from_min ) /
                            static_cast< U >( from_max - from_min ) );
}

/// Returns the size of the container, regardless of what it is
template < container Container >
[[nodiscard]] constexpr std::size_t cont_size( const Container& cont ) noexcept
{
        if constexpr ( static_sized< Container > )
                return std::tuple_size_v< Container >;
        else
                return std::size( cont );
}

/// Two items 'lh' and 'rh' are almost equal if their difference is smaller than
/// value 'eps'
template < typename T >
[[nodiscard]] constexpr bool
almost_equal( const T& lh, const T& rh, const float eps = default_epsilon )
{
        return float( std::abs( lh - rh ) ) < eps;
}

/// Returns range over Container, which skips first item of container
template < referenceable_container Container, typename Iterator = iterator_of_t< Container > >
[[nodiscard]] constexpr view< Iterator > tail( Container&& cont, const int step = 1 )
{
        return view< Iterator >( std::begin( cont ) + step, std::end( cont ) );
}

/// Returns range over Container, which skips last item of container
template < referenceable_container Container, typename Iterator = iterator_of_t< Container > >
[[nodiscard]] constexpr view< Iterator > init( Container&& cont, const int step = 1 )
{
        return view< Iterator >( std::begin( cont ), std::end( cont ) - step );
}

/// Returns iterator for first item, for which call to predicate f(*iter) holds true. end()
/// iterator is returned otherwise. The end() iterator is taken once, before the
/// container is iterated.
template <
    range_container                  Container,
    container_invocable< Container > PredicateCallable = std::identity >
[[nodiscard]] constexpr auto find_if( Container&& cont, PredicateCallable&& f = std::identity() )
{
        auto beg = std::begin( cont );
        auto end = std::end( cont );
        for ( ; beg != end; ++beg )
                if ( f( *beg ) )
                        return beg;
        return cont.end();
}

/// Returns index of an element in tuple 't', for which call to predicate f(x) holds true,
/// otherwise returns index of 'past the end' item - size of the tuple
template <
    gettable_container               Container,
    container_invocable< Container > PredicateCallable = std::identity >
requires( !range_container< Container > )
[[nodiscard]] constexpr std::size_t
find_if( Container&& t, PredicateCallable&& f = std::identity() )
{
        return impl::find_if_impl(
            std::forward< Container >( t ),
            std::forward< PredicateCallable >( f ),
            std::make_index_sequence< std::tuple_size_v< std::decay_t< Container > > >{} );
}

/// Finds first item in container 'cont' that is equal to 'item', returns
/// iterator for container, or index for tuples
template < container Container, typename T >
[[nodiscard]] constexpr auto find( Container&& cont, const T& item )
{
        return find_if( std::forward< Container >( cont ), [&]( const auto& sub_item ) {
                return sub_item == item;
        } );
}

/// Checks if container `cont` contains at least one occurence of `item`, returns true/false
template < container Container, typename T >
[[nodiscard]] constexpr bool contains( const Container& cont, const T& item )
{
        if constexpr ( range_container< Container > )
                return find( cont, item ) != cont.end();
        else
                return find( cont, item ) != cont_size( cont );
}

/// Applies unary callable 'f' to each element of container 'cont'
template < gettable_container Container, container_invocable< Container > UnaryCallable >
requires( !range_container< Container > )
constexpr void for_each( Container&& cont, UnaryCallable&& f )
{
        std::apply(
            [&f]< typename... Items >( Items&&... items ) {
                    ( f( std::forward< Items >( items ) ), ... );
            },
            std::forward< Container >( cont ) );
}

/// Applies unary callable 'f' to each element of container 'cont'
template < range_container Container, container_invocable< Container > UnaryCallable >
constexpr void for_each( Container&& cont, UnaryCallable&& f )
{
        for ( auto&& item : std::forward< Container >( cont ) )
                f( std::forward< decltype( item ) >( item ) );
}

/// Applies unary callable 'f(x)' to each element of container 'cont', returns
/// the largest and the smallest return value. of 'f(x)' calls. Returns the
/// default value of the 'f(x)' return type if container is empty.
template <
    container                        Container,
    container_invocable< Container > UnaryCallable = std::identity,
    typename T = std::decay_t< mapped_t< Container, UnaryCallable > > >
[[nodiscard]] constexpr min_max< T >
min_max_elem( Container&& cont, UnaryCallable&& f = std::identity() )
{
        min_max< T > res;
        res.max() = std::numeric_limits< T >::lowest();
        res.min() = std::numeric_limits< T >::max();

        for_each( cont, [&]( auto& item ) {
                auto val  = f( item );
                res.max() = std::max( res.max(), val );
                res.min() = std::min( res.min(), val );
        } );
        return res;
}

/// Applies unary callable 'f(x)' to each element of container 'cont', returns
/// the largest return value of 'f(x)' calls. Returns lowest value of the return
/// type if container is empty.
template <
    container                        Container,
    container_invocable< Container > UnaryCallable = std::identity,
    typename T = std::decay_t< mapped_t< Container, UnaryCallable > > >
[[nodiscard]] constexpr T max_elem( Container&& cont, UnaryCallable&& f = std::identity() )
{
        T val = std::numeric_limits< T >::lowest();
        for_each( cont, [&]( auto& item ) {
                val = std::max( f( item ), val );
        } );
        return val;
}

/// Applies unary callable 'f(x) to each element of container 'cont`, returns the
/// smallest return value of 'f(x)' calls. Returns maximum value of the return
/// type if container is empty.
template <
    container                        Container,
    container_invocable< Container > UnaryCallable = std::identity,
    typename T = std::decay_t< mapped_t< Container, UnaryCallable > > >
[[nodiscard]] constexpr T min_elem( Container&& cont, UnaryCallable&& f = std::identity() )
{
        T val = std::numeric_limits< T >::max();
        for_each( cont, [&]( auto& item ) {
                val = std::min( f( item ), val );
        } );
        return val;
}

/// Applies the predicate 'f(x)' to each element of container 'cont' and
/// returns the count of items, for which f(x) returned 'true'
template < container Container, container_invocable< Container > UnaryCallable = std::identity >
[[nodiscard]] constexpr std::size_t count( Container&& cont, UnaryCallable&& f = std::identity() )
{
        std::size_t res = 0;
        for_each( cont, [&]( auto& item ) {
                if ( f( item ) )
                        res += 1;
        } );
        return res;
}

/// Applies f(x) to each item of container 'cont', returns the sum of all the
/// return values of each call to 'f(x)' and 'init' item
template <
    container                        Container,
    container_invocable< Container > UnaryCallable = std::identity,
    typename T = std::decay_t< mapped_t< Container, UnaryCallable > > >
[[nodiscard]] constexpr T sum( Container&& cont, UnaryCallable&& f = std::identity(), T init = {} )
{
        for_each( cont, [&]( auto& item ) {
                init += f( item );
        } );
        return init;
}

/// Applies callable 'f(init,x)' to each element of container 'x' and actual
/// value of 'init' in iteration, returns a result of last application.
template < container Container, typename T, typename BinaryCallable >
[[nodiscard]] constexpr T accumulate( Container&& cont, T init, BinaryCallable&& f )
{
        for_each( cont, [&]( auto& item ) {
                init = f( std::move( init ), item );  /// NOLINT(bugprone-use-after-move)
        } );
        return init;
}

/// Applies callable 'f(x)' to each element of container 'cont' and returns the
/// average value of each call
template <
    container                        Container,
    container_invocable< Container > UnaryCallable = std::identity,
    typename T = std::decay_t< mapped_t< Container, UnaryCallable > > >
[[nodiscard]] constexpr T avg( Container&& cont, UnaryCallable&& f = std::identity() )
{
        T res{};
        for_each( cont, [&]( auto& item ) {
                res += f( item );
        } );
        if constexpr ( std::is_arithmetic_v< T > )
                return res / static_cast< T >( cont_size( cont ) );
        else
                return res / cont_size( cont );
}

/// Applies callable 'f(x)' to each element of container 'cont' and returns the
/// variance of values returned from the call. The `f` is applied twice to each element.
template <
    container                        Container,
    container_invocable< Container > UnaryCallable = std::identity,
    typename T = std::decay_t< mapped_t< Container, UnaryCallable > > >
[[nodiscard]] constexpr T variance( Container&& cont, UnaryCallable&& f = std::identity() )
{
        T u = avg( cont, f );

        T res = sum( cont, [&f, &u]( auto& val ) {
                auto v = f( val ) - u;
                return v * v;
        } );
        if constexpr ( std::is_arithmetic_v< T > )
                return res / static_cast< T >( cont_size( cont ) );
        else
                return res / cont_size( cont );
}

/// Applies binary callable 'f(x,y)' to each combination of items `x` from `lh_cont`
/// and `y` from `rh_cont`
template < container LhContainer, container RhContainer, typename BinaryCallable >
constexpr void for_cross_joint( LhContainer&& lh_cont, RhContainer&& rh_cont, BinaryCallable&& f )
{
        for_each( lh_cont, [&]( auto& lh_item ) {
                for_each( rh_cont, [&]( auto& rh_item ) {
                        f( lh_item, rh_item );
                } );
        } );
}

/// Returns true if call to predicate 'f(x)' returns true for at least one item `x` in
/// 'cont'
template < container Container, container_invocable< Container > PredicateCallable = std::identity >
[[nodiscard]] constexpr bool any_of( Container&& cont, PredicateCallable&& f = std::identity() )
{
        auto res = find_if( cont, std::forward< PredicateCallable >( f ) );

        if constexpr ( is_std_tuple_v< Container > )
                return res != std::tuple_size_v< std::decay_t< Container > >;
        else
                return res != cont.end();
}

/// Returns true if call to predicate 'f(x)' returns false for all items in
/// 'cont'.
template < container Container, container_invocable< Container > PredicateCallable = std::identity >
[[nodiscard]] constexpr bool none_of( Container&& cont, PredicateCallable&& f = std::identity() )
{
        return !any_of( cont, std::forward< PredicateCallable >( f ) );
}

/// Returns true if call to predicate 'f(x)' returns true for all items in 'cont'
template < container Container, container_invocable< Container > PredicateCallable = std::identity >
[[nodiscard]] constexpr bool all_of( Container&& cont, PredicateCallable&& f = std::identity() )
{
        return !any_of( cont, [&]( auto& item ) {
                return !f( item );
        } );
}

/// Returns true if containers 'lh' and 'rh' has same size and calls to predicate `f` -
/// `f(lh[i],rh[i])` return true for each item. Default callable is equality. all 0 <= i < size()
template <
    range_container LhContainer,
    range_container RhContainer,
    typename BinaryPredicateCallable = std::equal_to< void > >
[[nodiscard]] constexpr bool
equal( LhContainer&& lh, RhContainer&& rh, BinaryPredicateCallable&& f = std::equal_to< void >{} )
{
        if ( cont_size( lh ) != cont_size( rh ) )
                return false;
        auto rbeg = std::begin( rh );
        for ( auto& item : lh ) {
                if ( !f( item, *rbeg ) )
                        return false;
                ++rbeg;
        }
        return true;
}

/// Calls callable `f(x)` for each item in container 'cont' (or tuple) and stores
/// result in 'ResultContainer', which is returned. The  behavior depends on what kind of
/// 'ResultContainer' is used, rules are in this order:
///  1. std::array is constructed and `res[i] = f(cont[i])` is used for i = 0...N
///  2. if 'ResultContainer' has `push_back(x)` method, that is used to insert
///  result of calls to `f(x)`
///  3. `insert(x)` method is used to insert result of calls to `f(x)`
template <
    impl::map_f_collectable          ResultContainer,
    container                        Container,
    container_invocable< Container > UnaryCallable = std::identity >
[[nodiscard]] ResultContainer map_f( Container&& cont, UnaryCallable&& f = std::identity() )
{
        ResultContainer                          res{};
        impl::map_f_collector< ResultContainer > collector;

        for_each( std::forward< Container >( cont ), [&]< typename Item >( Item&& item ) {
                collector.collect( res, f( std::forward< Item >( item ) ) );
        } );
        return res;
}

/// Calls callable `f(cont[i])` for i = 0...N and stores the result in array of an
/// size N. The function requires size 'N' as template parameter
template <
    std::size_t                      N,
    range_container                  Container,
    container_invocable< Container > UnaryCallable = std::identity,
    typename T = std::decay_t< mapped_t< Container, UnaryCallable > > >
[[nodiscard]] std::array< T, N > map_f_to_a( Container&& cont, UnaryCallable&& f = std::identity() )
requires( !static_sized< Container > )
{
        return impl::map_f_to_a_impl< T, N >(
            std::forward< Container >( cont ),
            std::forward< UnaryCallable >( f ),
            std::make_index_sequence< N >() );
}

/// Calls callable f(cont[i]) for i = 0...N and stores the result in array of an
/// appropiate size. THe size `N` is infered from container.
template <
    gettable_container               Container,
    std::size_t                      N = std::tuple_size< std::decay_t< Container > >::value,
    container_invocable< Container > UnaryCallable = std::identity,
    typename T = std::decay_t< mapped_t< Container, UnaryCallable > > >
[[nodiscard]] std::array< T, N > map_f_to_a( Container&& cont, UnaryCallable&& f = std::identity() )
requires static_sized< Container >
{
        return impl::map_f_to_a_impl< T, N >(
            std::forward< Container >( cont ),
            std::forward< UnaryCallable >( f ),
            std::make_index_sequence< N >() );
}

/// Object with `operator()` that constructs object of type `T `out of passed-in value. Usefull for
/// for functions expecting callable as an argument.
template < typename T >
struct convert_to
{
        template < typename U >
        constexpr T operator()( U&& src ) const
            noexcept( noexcept( T{ std::forward< U >( src ) } ) )
        {
                return T{ std::forward< U >( src ) };
        }
};

/// Function applies callable `f` to each item in container `cont` and contacts results with
/// operator+, `val` is used as a separator between the items.
/// Returns `f(cont[0]) + val + f(cont[1]) + val + ... + val + f(cont[n]);`.
template <
    range_container Container,
    typename T,
    container_invocable< Container > UnaryCallable = std::identity >
[[nodiscard]] constexpr T
joined( Container&& cont, const T& val, UnaryCallable&& f = std::identity() )
{
        if ( cont.empty() )
                return T{};
        T res = f( *std::begin( cont ) );
        for ( auto& item : tail( cont ) )
                res += val + f( item );
        return res;
}

template < container Container, typename Iterator >
void copy( Container&& cont, Iterator iter )
{
        for_each( cont, [&]( auto& item ) {
                *iter = item;
                ++iter;
        } );
}

/// Executes unary callable `f()` with template argument of type 'std::size_t', which ranges from 0
/// to N.
template < std::size_t N, typename NullCallable >
constexpr void for_each_index( NullCallable&& f )
{
        if constexpr ( N != 0 ) {
                for_each_index< N - 1 >( f );
                f.template operator()< N - 1 >();
        }
}

/// Executes unary callable `f()` with template argument of type 'std::size_t', which ranges from 0
/// to N until first call that returns true. Function returns the index on which predicate returned
/// true.
template < std::size_t N, typename PredicateCallable >
constexpr std::size_t find_if_index( PredicateCallable&& f )
{
        std::size_t res = N;
        until_index< N >( [&f, &res]< std::size_t i >() {
                res = i;
                return f.template operator()< i >();
        } );
        return res;
}

/// Executes predicate `f()` with template argument of type 'std::size_t', which ranges from 0
/// to i until first call that returns true. Function returns whenever the `f` was called or not.
template < std::size_t i, typename PredicateCallable >
constexpr bool until_index( PredicateCallable&& f )
{
        if constexpr ( i != 0 )
                return until_index< i - 1 >( f ) || f.template operator()< i - 1 >();
        else
                return false;
}

/// Function expectes bounded value as index input and callable. Based on the value
/// of index, `f()` is called with template argument `std::size_t` with internal value of provided
/// index.
//
/// Expectes the bounded value to be valid (that is within the range)
template < bounded_derived IndexType, typename Callable >
requires( !requires( Callable f ) {
        { f.template operator()< 0 >() } -> std::same_as< void >;
} )
constexpr auto select_index( IndexType i, Callable&& f )
{
        using T = std::decay_t< decltype( f.template operator()< 0 >() ) >;
        T res{};
        select_index( i, [&res, &f]< std::size_t i >() {
                res = f.template operator()< i >();
        } );
        return res;
}

template < bounded_derived IndexType, typename Callable >
requires requires( Callable f ) {
        { f.template operator()< 0 >() } -> std::same_as< void >;
}
constexpr void select_index( IndexType i, Callable&& f )
{
        until_index< IndexType::max_val + 1 >( [&i, &f]< std::size_t j >() {
                if ( *i == j ) {
                        f.template operator()< j >();
                        return true;
                }
                return false;
        } );
}

/// Conveft the provided arguments into array of std::byte
template < typename... Args, std::size_t N = sizeof...( Args ) >
constexpr std::array< std::byte, N > bytes( const Args&... args )
{
        return std::array< std::byte, N >{ static_cast< std::byte >( args )... };
}

/// Expects multiple std::arrays on input, and merges all together into one std::array instance
template < typename Arr, typename... Arrs >
constexpr auto merge_arrays( Arr&& first, Arrs&&... arrs )
{
        using value_type = typename std::decay_t< Arr >::value_type;

        static_assert(
            ( std::convertible_to< typename std::decay_t< Arrs >::value_type, value_type > && ... &&
              true ),
            "All arrays have to provide similar types" );

        constexpr std::size_t size =
            ( std::tuple_size_v< std::decay_t< Arrs > > + ... +
              std::tuple_size_v< std::decay_t< Arr > > );

        auto f = [&]< std::size_t... Is >( std::index_sequence< Is... > ) {
                return std::array< value_type, size >{
                    impl::get_ith_item_from_arrays< Is >( first, arrs... )... };
        };
        return f( std::make_index_sequence< size >{} );
}

/// Constructs an array filled with value `x`
template < std::size_t N, typename T >
constexpr std::array< T, N > filled( const T& item )
{
        return map_f_to_a< N >( range( N ), [&]( auto ) -> T {
                return item;
        } );
}

}  // namespace emlabcpp
