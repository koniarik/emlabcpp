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

#include "emlabcpp/experimental/geom/point.h"
#include "emlabcpp/experimental/geom/pose.h"
#include "emlabcpp/experimental/geom/simplex.h"
#include "emlabcpp/experimental/geom/vector.h"

#pragma once

namespace emlabcpp
{

template < std::size_t N >
using triangle = simplex< point< N >, 2 >;

inline point< 3 > get_triangle_sphere_center( const triangle< 3 >& tri )
{
        const vector< 3 > ab     = tri[0] - tri[1];
        const vector< 3 > ac     = tri[0] - tri[2];
        const vector< 3 > normal = normalized( cross_product( ab, ac ) );
        vector< 3 >       p_ab   = cross_product( normal, ab );
        vector< 3 >       p_ac   = cross_product( normal, ac );
        vector< 3 >       c_ac   = ( vector_cast( tri[0] ) + vector_cast( tri[2] ) ) / 2;
        vector< 3 >       c_ab   = ( vector_cast( tri[0] ) + vector_cast( tri[1] ) ) / 2;

        float k;

        k = p_ab[0] * ( c_ab[1] - c_ac[1] ) + p_ab[1] * ( c_ac[0] - c_ab[0] );
        //-------------------------------------------------------------------
        k /= ( p_ab[0] * p_ac[1] - p_ac[0] * p_ab[1] );

        return point_cast( c_ac + k * p_ac );
}

constexpr vector< 3 > normal_of( const triangle< 3 >& tri )
{
        return cross_product( tri[0] - tri[1], tri[0] - tri[2] );
}

constexpr triangle< 3 > transform( const triangle< 3 >& t, const pose& transformation )
{
        return triangle< 3 >{
            transform( t[0], transformation ),
            transform( t[1], transformation ),
            transform( t[2], transformation ) };
}

template < std::size_t N >
constexpr triangle< N > scale( const triangle< N >& t, const point< N >& scales )
{
        return triangle< N >{ t[0] * scales, t[1] * scales, t[2] * scales };
}

}  // namespace emlabcpp
