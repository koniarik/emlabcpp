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
        vector< N > direction      = l[1] - l[0];
        float       length_squared = length2_of( direction );
        if ( length_squared == 0.f ) {
                return distance_of( p, l[0] );
        }
        float projection_dist = dot( p - l[0], direction );
        projection_dist /= length_squared;
        projection_dist = std::clamp( projection_dist, 0.f, 1.f );

        point< N > closest_p = l[0] + projection_dist * direction;

        return distance_of( closest_p, p );
}

template < std::size_t N >
constexpr float distance_of( const point< N >& p, const line< N >& line )
{
        return distance_of( line, p );
}

}  // namespace emlabcpp
