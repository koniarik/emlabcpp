/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
///

#pragma once

#include "emlabcpp/experimental/geom/vec_point_base.h"

namespace emlabcpp
{

template < std::size_t N >
class vector : public vec_point_base< vector< N >, N >
{
public:
        using vec_point_base< vector, N >::vec_point_base;

        constexpr vector< N >& operator+=( const vector< N >& other )
        {
                for ( const std::size_t i : range( N ) )
                        ( *this )[i] += other[i];
                return *this;
        }

        constexpr vector< N >& operator-=( const vector< N >& other )
        {
                for ( const std::size_t i : range( N ) )
                        ( *this )[i] -= other[i];
                return *this;
        }
};

/// instances of constants in the code for X/Y/Z axis
///
constexpr vector< 3 > x_axis{ 1, 0, 0 };
constexpr vector< 3 > y_axis{ 0, 1, 0 };
constexpr vector< 3 > z_axis{ 0, 0, 1 };

template < std::size_t N >
constexpr vector< N > operator+( vector< N > lh, const vector< N >& rh )
{
        return lh += rh;
}

/// Calculates cross product between points A and B
///
constexpr vector< 3 > cross_product( const vector< 3 >& a, const vector< 3 >& b )
{
        return vector< 3 >{
            a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0] };
}

/// Returns a normal to a point A in two dimensions
///
constexpr vector< 2 > normal_of( const vector< 2 >& a )
{
        return vector< 2 >{ a[1], -a[0] };
}

template < std::size_t N >
constexpr float vector_angle( const vector< N >& a, const vector< N >& b )
{
        auto s = sqrt( length2_of( a ) * length2_of( b ) );
        return acosf( float( dot( a, b ) ) / *s );
}

}  // namespace emlabcpp
