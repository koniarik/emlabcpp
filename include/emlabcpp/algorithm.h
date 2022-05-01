#include "emlabcpp/algorithm/impl.h"
#include "emlabcpp/bounded.h"
#include "emlabcpp/types.h"
#include "emlabcpp/view.h"

#include <cmath>
#include <cstdlib>
#include <tuple>

#pragma once

namespace emlabcpp
{

using std::abs;
using std::max;
using std::min;

constexpr float default_epsilon = 1.19e-07f;

/// Sometimes necessary to disable warnings of unused arguments
/// Hint: use std::ignore
template < typename T >
[[deprecated]] constexpr void ignore( T&& )
{
}

/// returns sign of variable T: -1,0,1
template < typename T >
constexpr int sign( T&& val )
{
        using value_type = std::decay_t< T >;
        if ( value_type{ 0 } > val ) {
                return -1;
        }
        if ( value_type{ 0 } < val ) {
                return 1;
        }
        return 0;
}

/// maps input value 'input' from input range to equivalent value in output range
template < arithmetic_operators T, arithmetic_operators U >
[[nodiscard]] constexpr U map_range( T input, T from_min, T from_max, U to_min, U to_max )
{
        return to_min + ( to_max - to_min ) * static_cast< U >( input - from_min ) /
                            static_cast< U >( from_max - from_min );
}

/// Returns the size of the container, regardless of what it is
template < container Container >
[[nodiscard]] constexpr std::size_t cont_size( const Container& cont ) noexcept
{
        if constexpr ( static_sized< Container > ) {
                return std::tuple_size_v< Container >;
        } else {
                return cont.size();
        }
}

/// two items 'lh' and 'rh' are almost equal if their difference is smaller than
/// value 'eps'
template < typename T >
[[nodiscard]] constexpr bool almost_equal( const T& lh, const T& rh, float eps = default_epsilon )
{
        return float( abs( lh - rh ) ) < eps;
}

/// Returns range over Container, which skips first item of container
template < referenceable_container Container, typename Iterator = iterator_of_t< Container > >
[[nodiscard]] constexpr view< Iterator > tail( Container&& cont, int step = 1 )
{
        return view< Iterator >( std::begin( cont ) + step, std::end( cont ) );
}

/// Returns range over Container, which skips last item of container
template < referenceable_container Container, typename Iterator = iterator_of_t< Container > >
[[nodiscard]] constexpr view< Iterator > init( Container&& cont, int step = 1 )
{
        return view< Iterator >( std::begin( cont ), std::end( cont ) - step );
}

/// Returns iterator for first item, for which call to f(*iter) holds true. end()
/// iterator is returned otherwise. The end() iterator is taken once, before the
/// container is iterated.
template <
    range_container                  Container,
    container_invocable< Container > UnaryFunction = std::identity >
[[nodiscard]] constexpr auto find_if( Container&& cont, UnaryFunction&& f = std::identity() )
{
        auto beg = std::begin( cont );
        auto end = std::end( cont );
        for ( ; beg != end; ++beg ) {
                if ( f( *beg ) ) {
                        return beg;
                }
        }
        return cont.end();
}

/// Returns index of an element in tuple 't', for which call to f(x) holds true,
/// otherwise returns index of 'past the end' item - size of the tuple
template <
    gettable_container               Container,
    container_invocable< Container > UnaryFunction = std::identity >
requires( !range_container< Container > ) [[nodiscard]] constexpr std::size_t
    find_if( Container&& t, UnaryFunction&& f = std::identity() )
{
        return impl::find_if_impl(
            t,
            std::forward< UnaryFunction >( f ),
            std::make_index_sequence< std::tuple_size_v< std::decay_t< Container > > >{} );
}

/// Finds first item in container 'cont' that is equal to 'item', returns
/// iterator for container and index for tuples
template < container Container, typename T >
[[nodiscard]] constexpr auto find( Container&& cont, const T& item )
{
        return find_if( cont, [&]( const auto& sub_item ) {
                return sub_item == item;
        } );
}

/// Applies unary function 'f' to each element of container 'cont'
template < gettable_container Container, container_invocable< Container > UnaryFunction >
requires(
    !range_container< Container > ) constexpr void for_each( Container&& cont, UnaryFunction&& f )
{
        std::apply(
            [&]< typename... Items >( Items&&... items ) {
                    ( f( std::forward< Items >( items ) ), ... );
            },
            std::forward< Container >( cont ) );
}

/// Applies unary function 'f' to each element of container 'cont'
template < range_container Container, container_invocable< Container > UnaryFunction >
constexpr void for_each( Container&& cont, UnaryFunction&& f )
{
        for ( auto&& item : std::forward< Container >( cont ) ) {
                f( std::forward< decltype( item ) >( item ) );
        }
}

/// Helper structure for finding the smallest and the largest item in some
/// container, contains min/max attributes representing such elements.
template < typename T >
struct min_max
{
        using value_type = T;

        T min{};
        T max{};

        min_max() = default;
        min_max( T min_i, T max_i )
          : min( std::move( min_i ) )
          , max( std::move( max_i ) )
        {
        }
};

/// Applies unary function 'f(x)' to each element of container 'cont', returns
/// the largest and the smallest return value. of 'f(x)' calls. Returns the
/// default value of the 'f(x)' return type if container is empty.
template <
    container                        Container,
    container_invocable< Container > UnaryFunction = std::identity,
    typename T = std::decay_t< mapped_t< Container, UnaryFunction > > >
[[nodiscard]] constexpr min_max< T >
min_max_elem( const Container& cont, UnaryFunction&& f = std::identity() )
{
        min_max< T > res;
        res.max = std::numeric_limits< T >::lowest();
        res.min = std::numeric_limits< T >::max();

        for_each( cont, [&]( const auto& item ) {
                auto val = f( item );
                res.max  = max( res.max, val );
                res.min  = min( res.min, val );
        } );
        return res;
}

/// Applies unary function 'f(x)' to each element of container 'cont', returns
/// the largest return value of 'f(x)' calls. Returns lowest value of the return
/// type if container is empty.
template <
    container                        Container,
    container_invocable< Container > UnaryFunction = std::identity,
    typename T = std::decay_t< mapped_t< Container, UnaryFunction > > >
[[nodiscard]] constexpr T max_elem( const Container& cont, UnaryFunction&& f = std::identity() )
{
        T val = std::numeric_limits< T >::lowest();
        for_each( cont, [&]( const auto& item ) {
                val = max( f( item ), val );
        } );
        return val;
}

/// Applies unary function 'f(x) to each element of container 'cont, returns the
/// smallest return value of 'f(x)' calls. Returns maximum value of the return
/// type if container is empty.
template <
    container                        Container,
    container_invocable< Container > UnaryFunction = std::identity,
    typename T = std::remove_reference_t< mapped_t< Container, UnaryFunction > > >
[[nodiscard]] constexpr T min_elem( const Container& cont, UnaryFunction&& f = std::identity() )
{
        T val = std::numeric_limits< T >::max();
        for_each( cont, [&]( const auto& item ) {
                val = min( f( item ), val );
        } );
        return val;
}

/// Applies the unary function 'f(x)' to each element of container 'cont' and
/// returns the count of items, for which f(x) returned 'true'
template < container Container, container_invocable< Container > UnaryFunction = std::identity >
[[nodiscard]] constexpr std::size_t
count( const Container& cont, UnaryFunction&& f = std::identity() )
{
        std::size_t res = 0;
        for_each( cont, [&]( const auto& item ) {
                if ( f( item ) ) {
                        res += 1;
                }
        } );
        return res;
}

/// Applies f(x) to each item of container 'cont', returns the sum of all the
/// return values of each call to 'f(x)' and 'init' item
template <
    container                        Container,
    container_invocable< Container > UnaryFunction = std::identity,
    typename T = std::decay_t< mapped_t< Container, UnaryFunction > > >
[[nodiscard]] constexpr T
sum( const Container& cont, UnaryFunction&& f = std::identity(), T init = {} )
{
        for_each( cont, [&]( const auto& item ) {
                init += f( item );
        } );
        return init;
}

/// Applies function 'f(init,x)' to each element of container 'x' and actual
/// value of 'init' in iteration, the return value is 'init' value for next round
template < container Container, typename T, typename BinaryFunction >
[[nodiscard]] constexpr T accumulate( const Container& cont, T init, BinaryFunction&& f )
{
        for_each( cont, [&]( const auto& item ) {
                init = f( std::move( init ), item );  /// NOLINT(bugprone-use-after-move)
        } );
        return init;
}

/// Applies function 'f(x)' to each element of container 'cont' and returns the
/// average value of each call
template <
    container                        Container,
    container_invocable< Container > UnaryFunction = std::identity,
    typename T = std::decay_t< mapped_t< Container, UnaryFunction > > >
[[nodiscard]] constexpr T avg( const Container& cont, UnaryFunction&& f = std::identity() )
{
        T res{};
        for_each( cont, [&]( const auto& item ) {
                res += f( item );
        } );
        if constexpr ( std::is_arithmetic_v< T > ) {
                return res / static_cast< T >( cont_size( cont ) );
        } else {
                return res / cont_size( cont );
        }
}

/// Applies function 'f(x)' to each element of container 'cont' and returns the
/// variance of values returned from the call. The `f` is applied twice to each element.
template <
    container                        Container,
    container_invocable< Container > UnaryFunction = std::identity,
    typename T = std::decay_t< mapped_t< Container, UnaryFunction > > >
[[nodiscard]] constexpr T variance( const Container& cont, UnaryFunction&& f = std::identity() )
{
        T u = avg( cont, f );

        T res = sum( cont, [&]( const auto& val ) {
                auto v = f( val ) - u;
                return v * v;
        } );
        if constexpr ( std::is_arithmetic_v< T > ) {
                return res / static_cast< T >( cont_size( cont ) );
        } else {
                return res / cont_size( cont );
        }
}

/// Applies binary function 'f(x,y)' to each combination of items x in lh_cont
/// and y in rh_cont
template < container LhContainer, container RhContainer, typename BinaryFunction >
constexpr void for_cross_joint( LhContainer&& lh_cont, RhContainer&& rh_cont, BinaryFunction&& f )
{
        for_each( lh_cont, [&]( auto& lh_item ) {
                for_each( rh_cont, [&]( auto& rh_item ) {
                        f( lh_item, rh_item );
                } );
        } );
}

/// Returns true if call to function 'f(x)' returns true for at least one item in
/// 'cont'
template < container Container, container_invocable< Container > UnaryFunction = std::identity >
[[nodiscard]] constexpr bool any_of( const Container& cont, UnaryFunction&& f = std::identity() )
{
        auto res = find_if( cont, std::forward< UnaryFunction >( f ) );

        if constexpr ( is_std_tuple_v< Container > ) {
                return res != std::tuple_size_v< Container >;
        } else {
                return res != cont.end();
        }
}

/// Returns true if call to function 'f(x)' returns false for all items in
/// 'cont'.
template < container Container, container_invocable< Container > UnaryFunction = std::identity >
[[nodiscard]] constexpr bool none_of( const Container& cont, UnaryFunction&& f = std::identity() )
{
        return !any_of( cont, std::forward< UnaryFunction >( f ) );
}

/// Returns true if call to function 'f(x)' returns true for all items in 'cont'
template < container Container, container_invocable< Container > UnaryFunction = std::identity >
[[nodiscard]] constexpr bool all_of( const Container& cont, UnaryFunction&& f = std::identity() )
{
        return !any_of( cont, [&]( const auto& item ) {
                return !f( item );
        } );
}

/// Returns true of containers 'lh' and 'rh' has same size and lh[i] == rh[i] for
/// all 0 <= i < size()
template < range_container LhContainer, range_container RhContainer >
[[nodiscard]] constexpr bool equal( const LhContainer& lh, const RhContainer& rh )
{
        if ( lh.size() != rh.size() ) {
                return false;
        }
        auto lbeg = std::begin( lh );
        auto rbeg = std::begin( rh );
        auto lend = std::end( lh );
        for ( ; lbeg != lend; ++lbeg, ++rbeg ) {
                if ( *lbeg != *rbeg ) {
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
template <
    impl::map_f_collectable          ResultContainer,
    container                        Container,
    container_invocable< Container > UnaryFunction = std::identity >
[[nodiscard]] inline ResultContainer map_f( Container&& cont, UnaryFunction&& f = std::identity() )
{
        ResultContainer                          res{};
        impl::map_f_collector< ResultContainer > collector;

        for_each( std::forward< Container >( cont ), [&]< typename Item >( Item&& item ) {
                collector.collect( res, f( std::forward< Item >( item ) ) );
        } );
        return res;
}

/// Calls function f(cont[i]) for i = 0...N and stores the result in array of an
/// appropiate size. The functions needs size 'N' as template parameter
template <
    std::size_t                      N,
    range_container                  Container,
    container_invocable< Container > UnaryFunction = std::identity,
    typename T = std::decay_t< mapped_t< Container, UnaryFunction > > >
[[nodiscard]] inline std::array< T, N > map_f_to_a(
    Container&&     cont,
    UnaryFunction&& f = std::identity() ) requires( !static_sized< Container > )
{
        return impl::map_f_to_a_impl< T, N >(
            std::forward< Container >( cont ),
            std::forward< UnaryFunction >( f ),
            std::make_index_sequence< N >() );
}

/// Calls function f(cont[i]) for i = 0...N and stores the result in array of an
/// appropiate size.
template <
    gettable_container               Container,
    std::size_t                      N = std::tuple_size< std::decay_t< Container > >::value,
    container_invocable< Container > UnaryFunction = std::identity,
    typename T = std::decay_t< mapped_t< Container, UnaryFunction > > >
[[nodiscard]] inline std::array< T, N > map_f_to_a(
    Container&&     cont,
    UnaryFunction&& f = std::identity() ) requires static_sized< Container >
{
        return impl::map_f_to_a_impl< T, N >(
            std::forward< Container >( cont ),
            std::forward< UnaryFunction >( f ),
            std::make_index_sequence< N >() );
}

/// object with operator() that constructs object of type T out of passed-in value. Usefull for
/// functions like `map_f`
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

/// Returns cont[0] + val + cont[1] + val + cont[2] + ... + cont[n-1] + val +
/// cont[n];
template < range_container Container, typename T >
[[nodiscard]] constexpr T joined( const Container& cont, T&& val )
{
        if ( cont.empty() ) {
                return T{};
        }
        T res = *std::begin( cont );
        for ( const auto& item : tail( cont ) ) {
                res += val + item;
        }
        return res;
}

/// Executes unary function f() with template argument of type 'std::size_t', which ranges from 0 to
/// i.
template < std::size_t i, typename NullFunction >
constexpr void for_each_index( NullFunction&& f )
{
        if constexpr ( i != 0 ) {
                for_each_index< i - 1 >( f );
                f.template operator()< i - 1 >();
        }
}

/// Executes unary predicate f() with template argument of type 'std::size_t', which ranges from 0 to
/// i until first call that returns true. Function returns whenever the f was called or not.
template < std::size_t i, typename PredFunction >
constexpr bool until_index( PredFunction&& f )
{
        if constexpr ( i != 0 ) {
                return until_index< i - 1 >( f ) || f.template operator()< i - 1 >();
        } else {
                return false;
        }
}

/// Function expectes bounded value as index input and callable nullary function. Based on the value
/// of index, f() is called with template argument std::size_t with internal value of the provided
/// index.
//
/// Expectes the bounded value to be valid (that is within the range)
template < bounded_derived IndexType, typename NullFunction >
requires( !requires( NullFunction f ) {
        {
                f.template operator()< 0 >()
                } -> std::same_as< void >;
} ) constexpr auto select_index( IndexType i, NullFunction&& f )
{
        using T = std::decay_t< decltype( f.template operator()< 0 >() ) >;
        T res{};
        select_index( i, [&]< std::size_t i >() {
                res = f.template operator()< i >();
        } );
        return res;
}

template < bounded_derived IndexType, typename NullFunction >
requires requires( NullFunction f )
{
        {
                f.template operator()< 0 >()
                } -> std::same_as< void >;
}
constexpr void select_index( IndexType i, NullFunction&& f )
{
        until_index< IndexType::max_val + 1 >( [&]< std::size_t j >() {
                if ( *i == j ) {
                        f.template operator()< j >();
                        return true;
                }
                return false;
        } );
}

}  /// namespace emlabcpp
