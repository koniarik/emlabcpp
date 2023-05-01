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
#include "emlabcpp/experimental/geom/vector.h"

#include <array>

#pragma once

namespace emlabcpp
{

// represents an set of two points, type alies of std::array because nothing more specific is
// required
template < std::size_t N >
using line = std::array< point< N >, 2 >;

// calculate shortest distance between any point on line and point 'p'
template < std::size_t N >
constexpr float distance_of( const line< N >& l, const point< N >& p )
{
        const vector< N > direction      = l[1] - l[0];
        const float       length_squared = length2_of( direction );
        if ( length_squared == 0.f ) {
                return distance_of( p, l[0] );
        }
        float projection_dist = dot( p - l[0], direction );
        projection_dist /= length_squared;
        projection_dist = std::clamp( projection_dist, 0.f, 1.f );

        const point< N > closest_p = l[0] + projection_dist * direction;

        return distance_of( closest_p, p );
}

template < std::size_t N >
constexpr float distance_of( const point< N >& p, const line< N >& line )
{
        return distance_of( line, p );
}

}  // namespace emlabcpp
