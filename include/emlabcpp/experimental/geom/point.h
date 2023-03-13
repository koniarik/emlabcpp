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

#include "emlabcpp/experimental/geom/vec_point_base.h"
#include "emlabcpp/experimental/geom/vector.h"
#include "emlabcpp/range.h"

#pragma once

namespace emlabcpp
{

/// Class implementing multidimensional point in coordinate system of dimension N
template < std::size_t N >
class point : public vec_point_base< point< N >, N >
{
public:
        using vec_point_base< point, N >::vec_point_base;

        /// += operator adds value of 'i'th coordinate from 'other' to 'this', for all 0 <= i < N
        constexpr point< N >& operator+=( const vector< N >& other )
        {
                for ( std::size_t i : range( N ) ) {
                        ( *this )[i] += other[i];
                }
                return *this;
        }

        /// -= operator subtracts value of 'i'th coordinate of 'other' from 'this', for all 0 <= i <
        /// N
        constexpr point< N >& operator-=( const vector< N >& other )
        {
                for ( std::size_t i : range( N ) ) {
                        ( *this )[i] -= other[i];
                }
                return *this;
        }

        ~point() = default;
};

template < std::size_t N >
constexpr point< N > point_cast( const vector< N >& v )
{
        return point< N >{ *v };
}

template < std::size_t N >
constexpr vector< N > vector_cast( const point< N >& p )
{
        return vector< N >{ *p };
}

/// Multiplication of points multiplies each coordinate of A by coordinate of B on same dimension
template < std::size_t N >
[[deprecated]] constexpr point< N > operator*( point< N > a, const point< N >& b )
{
        for ( std::size_t i : range( N ) ) {
                a[i] *= b[i];
        }
        return a;
}

/// Returns a result of subtraction of A from B, viz -= operator
///
template < std::size_t N >
constexpr vector< N > operator-( point< N > a, const point< N >& b )
{
        a -= vector_cast( b );
        return vector< N >{ *a };
}

/// Returns a result of addition a to b, viz += operator
///
template < std::size_t N >
constexpr point< N > operator+( point< N > a, const vector< N >& b )
{
        a += b;
        return a;
}

/// Returns a result of subtraction a to b, viz += operator
///
template < std::size_t N >
constexpr point< N > operator-( point< N > a, const vector< N >& b )
{
        a -= b;
        return a;
}

/// Returns euclidian distance of point A from point B
///
template < std::size_t N >
constexpr float distance_of( const point< N >& a, const point< N >& b )
{
        auto tmp = sum( range( N ), [&]( std::size_t i ) {
                return std::pow( a[i] - b[i], 2 );
        } );
        return std::sqrt( float( tmp ) );
}

template < std::size_t N >
constexpr float point_angle( const point< N >& a, const point< N >& b )
{
        return vector_angle( vector_cast( a ), vector_cast( b ) );
}

template < std::size_t N >
inline std::vector< point< N > >
lineary_interpolate_path( const std::vector< point< N > >& ipath, float d_step )
{
        std::vector< point< N > > res;
        if ( ipath.empty() ) {
                return res;
        }
        for ( std::size_t i : range( ipath.size() - 1 ) ) {
                const point< N >& from = ipath[i];
                const point< N >& to   = ipath[i + 1];

                std::size_t seg_steps = std::size_t{ distance_of( from, to ) / d_step };
                for ( std::size_t j : range( seg_steps ) ) {
                        res.push_back( lin_interp( from, to, float( j ) / float( seg_steps ) ) );
                }
        }
        res.push_back( ipath.back() );
        return res;
}

/// Function to calculate distance of projection of point A. That point is projected on axis defined
/// only by it's direction - 'axis_direction'. The distance of that projection from the [0,0,0]
/// coordinate is returned.
///
template < std::size_t N >
constexpr float axis_projection_distance( const point< N >& a, const vector< N >& axis_direction )
{
        return float( dot( vector_cast( a ), axis_direction ) );
}
}  // namespace emlabcpp
