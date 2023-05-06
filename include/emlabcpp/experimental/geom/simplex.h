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
#include "emlabcpp/experimental/geom/point.h"
#include "emlabcpp/experimental/matrix.h"

#include <algorithm>
#include <array>
#include <vector>

#pragma once

namespace emlabcpp
{

template < typename Item, std::size_t N >
class simplex
{
public:
        using container = std::array< Item, N + 1 >;

private:
        container points_;

public:
        using value_type     = Item;
        using iterator       = typename container::iterator;
        using const_iterator = typename container::const_iterator;

        constexpr explicit simplex() = default;

        constexpr explicit simplex( Item item )
          : points_( { item } )
        {
        }

        constexpr simplex( const simplex< Item, N - 1 >& sub, Item item )
          : points_()
        {
                std::copy( sub.begin(), sub.end(), points_.begin() );
                *points_.rbegin() = item;

                std::sort( points_.begin(), points_.end() );
        }

        constexpr simplex( std::array< Item, N + 1 > data )
          : points_( std::move( data ) )
        {
                std::sort( points_.begin(), points_.end() );
        }

        template < typename... Ts >
        constexpr explicit simplex( Ts... ts )
          : points_( { ts... } )
        {
                static_assert(
                    sizeof...( Ts ) == N + 1, "Number of parameters for simplex has to be N+1" );
        }

        [[nodiscard]] constexpr const_iterator begin() const
        {
                return points_.begin();
        }

        [[nodiscard]] constexpr const_iterator end() const
        {
                return points_.end();
        }

        [[nodiscard]] constexpr const Item& operator[]( std::size_t index ) const
        {
                return points_[index];
        }

        [[nodiscard]] constexpr std::size_t size() const
        {
                return points_.size();
        }
};

template < std::size_t N, std::size_t U >
constexpr point< N > center_of( const simplex< point< N >, U >& s )
{
        vector< N > avg = sum( s, [&]( const point< N >& p ) -> vector< N > {
                return vector_cast( p ) / s.size();
        } );

        return point_cast( avg );
}

template < std::size_t N >
constexpr float volume_of( const simplex< point< N >, N >& simplex )
{
        matrix< N, N > m;
        for ( const std::size_t i : range( simplex.size() - 1 ) ) {
                auto diff = simplex[i + 1] - simplex[0];
                std::copy( diff.begin(), diff.end(), &m[i][0] );
        }

        return std::abs( ( 1 / std::tgammaf( N + 1 ) ) * determinant( m ) );
}

template < typename Item, std::size_t N >
constexpr bool operator<( const simplex< Item, N >& lh, const simplex< Item, N >& rh )
{
        // there are actually N+1 items in N simplex;
        std::size_t i = *find_if( range( N ), [&]( std::size_t j ) {
                return lh[j] != rh[j];
        } );
        return lh[i] < rh[i];
}

template < typename Item, std::size_t N >
constexpr bool operator>( const simplex< Item, N >& lh, const simplex< Item, N >& rh )
{
        return rh < lh;
}

template < typename Item, std::size_t N >
constexpr bool operator<=( const simplex< Item, N >& lh, const simplex< Item, N >& rh )
{
        return !( lh > rh );
}

template < typename Item, std::size_t N >
constexpr bool operator>=( const simplex< Item, N >& lh, const simplex< Item, N >& rh )
{
        return !( lh < rh );
}

template < typename Item, std::size_t N >
constexpr bool operator==( const simplex< Item, N >& lh, const simplex< Item, N >& rh )
{
        return equal( lh, rh );
}

template < typename Item, std::size_t N >
constexpr bool operator!=( const simplex< Item, N >& lh, const simplex< Item, N >& rh )
{
        return !( lh == rh );
}

}  // namespace emlabcpp
