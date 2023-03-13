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

#include "emlabcpp/algorithm.h"
#include "emlabcpp/physical_quantity.h"
#include "emlabcpp/range.h"

#pragma once

namespace emlabcpp
{

template < typename Derived, std::size_t N >
class vec_point_base
{
public:
        using container = std::array< float, N >;

private:
        container data_ = { 0 };

        [[nodiscard]] Derived& impl()
        {
                return static_cast< Derived& >( *this );
        }
        [[nodiscard]] Derived const& impl() const
        {
                return static_cast< Derived const& >( *this );
        }

public:
        static constexpr std::size_t dimensions = N;
        using value_type                        = float;
        using const_iterator                    = typename container::const_iterator;
        using iterator                          = typename container::iterator;

        static Derived make_filled_with( value_type val )
        {
                std::array< float, N > res;
                for ( std::size_t i : range( N ) ) {
                        res[i] = val;
                }
                return Derived{ res };
        }

        constexpr vec_point_base() noexcept = default;

        explicit constexpr vec_point_base( container cont ) noexcept
          : data_( std::move( cont ) )
        {
        }

        template < typename... T >
        explicit constexpr vec_point_base( T... t )
          : data_( { float( t )... } )
        {
                static_assert( sizeof...( T ) == N, "Number of parameters has to be N" );
        }

        constexpr const_iterator begin() const
        {
                return data_.begin();
        }

        constexpr const_iterator end() const
        {
                return data_.end();
        }

        constexpr iterator begin()
        {
                return data_.begin();
        }

        constexpr iterator end()
        {
                return data_.end();
        }

        constexpr float operator[]( std::size_t i ) const
        {
                return data_[i];
        }

        constexpr float& operator[]( std::size_t i )
        {
                return data_[i];
        }

        constexpr Derived operator-() const
        {
                return { impl() * -1.f };
        }

        constexpr std::size_t size() const
        {
                return N;
        }

        constexpr const container& operator*() const
        {
                return data_;
        }

        friend constexpr auto operator<=>( const vec_point_base&, const vec_point_base& ) = default;
};

namespace detail
{
        template < typename Derived, std::size_t N >
        constexpr bool vec_point_derived_test( const vec_point_base< Derived, N >& )
        {
                return true;
        }
}  // namespace detail

template < typename T >
concept vec_point_derived = requires( T val )
{
        detail::vec_point_derived_test( val );
};

/** Multiplies each coordinate of A by item 's' of type T, if T satifies std::is_arithmetic
 */
template <
    typename Derived,
    std::size_t N,
    typename T,
    typename = typename std::enable_if_t< std::is_arithmetic_v< T > > >
constexpr Derived operator*( const vec_point_base< Derived, N >& a, T s )
{
        Derived res{ *a };
        for ( std::size_t i : range( N ) ) {
                res[i] *= s;
        }
        return res;
}
/** Multiplies each coordinate of A by item 's' of type T, if T satifies std::is_arithmetic
 */
template < typename Derived, std::size_t N, typename T >
constexpr Derived operator*( T s, const vec_point_base< Derived, N >& a )
{
        return a * s;
}

/** Divides each coordinate of A by item 's' of type T, if T satifies std::is_arithmetic
 */
template <
    typename Derived,
    std::size_t N,
    typename T,
    typename = typename std::enable_if_t< std::is_arithmetic_v< T > > >
constexpr Derived operator/( const vec_point_base< Derived, N >& a, T s )
{
        Derived res{ *a };
        for ( std::size_t i : range( N ) ) {
                res[i] /= float( s );
        }
        return res;
}
/** Calculates the dot product between A and B
 */
template < typename Derived, std::size_t N >
constexpr float dot( const vec_point_base< Derived, N >& a, const vec_point_base< Derived, N >& b )
{
        return sum( range( N ), [&]( std::size_t i ) {
                return a[i] * b[i];
        } );
}

/** Returns squared distance of A from [0,0,0], this is a squared length of vector represented
 ** by A
 */
template < typename Derived, std::size_t N >
constexpr auto length2_of( const vec_point_base< Derived, N >& a )
{
        auto res = sum( range( N ), [a]( std::size_t i ) {
                return std::pow( a[i], 2 );
        } );
        return float( res );
}

/** Returns distance of A from [0,0,0], this is a length of vector represented by A
 */
template < typename Derived, std::size_t N >
constexpr float length_of( const vec_point_base< Derived, N >& a )
{
        return std::sqrt( length2_of( a ) );
}

/** Calculates normalized version of A, this means that length(A) equals to 1
 */
template < typename Derived, std::size_t N >
constexpr Derived normalized( const vec_point_base< Derived, N >& a )
{
        return a / length_of( a );
}

/** Creates absolute version of A - removing signs on all dimensions
 */
template < typename Derived, std::size_t N >
constexpr Derived abs( const vec_point_base< Derived, N >& a )
{
        Derived res;
        for ( std::size_t i : range( N ) ) {
                res[i] = std::abs( a[i] );
        }
        return res;
}
/** Checks if A and B are equal within specified tolerance, this means that difference of all
 * coordinates of A and B has to be within that epsilon
 */
template < vec_point_derived Derived >
constexpr bool almost_equal( const Derived& a, const Derived& b, float eps = default_epsilon )
{
        constexpr std::size_t N = Derived::dimensions;
        return all_of( range( N ), [&]( std::size_t i ) {
                return almost_equal( a[i], b[i], eps );
        } );
}

template < typename Derived, std::size_t N >
constexpr Derived
max( const vec_point_base< Derived, N >& a, const vec_point_base< Derived, N >& b )
{
        return Derived{ a > b ? *a : *b };
}

template < typename Derived, std::size_t N >
constexpr const Derived&
min( const vec_point_base< Derived, N >& a, const vec_point_base< Derived, N >& b )
{
        return Derived{ a < b ? *a : *b };
}
/** Calculates a C, where C[i] = max(A[i], B[i]) holds for 0 <= i < N
 */
template < typename Derived, std::size_t N >
constexpr Derived
dimensional_max( const vec_point_base< Derived, N >& a, const vec_point_base< Derived, N >& b )
{
        Derived res;
        for ( std::size_t i : range( N ) ) {
                res[i] = std::max( a[i], b[i] );
        }
        return res;
}

/** Calculates a C, where C[i] = min(A[i], B[i]) holds for 0 <= i < N
 */
template < typename Derived, std::size_t N >
constexpr Derived
dimensional_min( const vec_point_base< Derived, N >& a, const vec_point_base< Derived, N >& b )
{
        Derived res;
        for ( std::size_t i : range( N ) ) {
                res[i] = std::min( a[i], b[i] );
        }
        return res;
}

template <
    typename Container,
    typename UnaryFunction = std::identity,
    typename Derived       = std::decay_t< mapped_t< Container, UnaryFunction > > >
constexpr min_max< Derived >
dimensional_min_max_elem( const Container& cont, UnaryFunction&& f = std::identity{} )
{
        min_max< Derived > bound_cube;
        bound_cube.min = std::numeric_limits< Derived >::max();
        bound_cube.max = std::numeric_limits< Derived >::lowest();
        for ( const auto& item : cont ) {
                const auto& p  = f( item );
                bound_cube.min = dimensional_min( bound_cube.min, p );
                bound_cube.max = dimensional_max( bound_cube.max, p );
        }
        return bound_cube;
}

template < typename Derived, std::size_t N >
constexpr Derived lin_interp(
    const vec_point_base< Derived, N >& from,
    const vec_point_base< Derived, N >& goal,
    float                               factor )
{
        Derived res;
        for ( std::size_t i : range( N ) ) {
                res[i] = from[i] + ( goal[i] - from[i] ) * factor;
        }
        return res;
}

}  // namespace emlabcpp
