#include "emlabcpp/experimental/geom/point.h"
#include "emlabcpp/experimental/geom/pose.h"
#include "emlabcpp/experimental/geom/simplex.h"

#pragma once

namespace emlabcpp
{

template < std::size_t N >
using triangle = simplex< point< N >, 2 >;

inline point< 3 > get_triangle_sphere_center( const triangle< 3 >& tri )
{
        vec< 3 > ab     = tri[0] - tri[1];
        vec< 3 > ac     = tri[0] - tri[2];
        vec< 3 > normal = normalized( cross_product( ab, ac ) );
        vec< 3 > p_ab   = cross_product( normal, ab );
        vec< 3 > p_ac   = cross_product( normal, ac );
        vec< 3 > c_ac   = ( vector_cast( tri[0] ) + vector_cast( tri[2] ) ) / 2;
        vec< 3 > c_ab   = ( vector_cast( tri[0] ) + vector_cast( tri[1] ) ) / 2;

        float k;

        k = p_ab[0] * ( c_ab[1] - c_ac[1] ) + p_ab[1] * ( c_ac[0] - c_ab[0] );
        //-------------------------------------------------------------------
        k /= ( p_ab[0] * p_ac[1] - p_ac[0] * p_ab[1] );

        return point_cast( c_ac + k * p_ac );
}

constexpr vec< 3 > normal_of( const triangle< 3 >& tri )
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
