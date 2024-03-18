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

#include "emlabcpp/experimental/geom/point.h"
#include "emlabcpp/experimental/geom/quaternion.h"

#include <utility>

namespace emlabcpp
{

/// distance between two poses in space, represented as 'space distance' and 'angular distance'
struct pose_distance
{
        float dist;
        float angle_dist;
};

/// returns steps necessary for linear interpolation of distance between poses 'dis', such that:
///  - the number of steps is minimized
///  - space distance between interpolated poses is smaller than dist_step
///  - angle distance between interpolated poses is smaller than angle_step

constexpr std::size_t steps( pose_distance dist, float dist_step, float angle_step )
{
        auto d_steps = std::size_t( 1.f + dist.dist / dist_step );
        auto a_steps = std::size_t( 1.f + dist.angle_dist / angle_step );
        return std::max( d_steps, a_steps );
}

/// represents orientation and position in 3D space
struct pose
{

        point< 3 > position;
        quaternion orientation;

        constexpr pose()
          : position( 0, 0, 0 )
          , orientation( neutral_quat )
        {
        }

        explicit constexpr pose( point< 3 > const& position )
          : position( position )
          , orientation( neutral_quat )
        {
        }

        explicit constexpr pose( quaternion const& orientation )
          : position( 0, 0, 0 )
          , orientation( orientation )
        {
        }

        constexpr pose( point< 3 > const& position, quaternion const& orientation )
          : position( position )
          , orientation( orientation )
        {
        }
};

constexpr bool operator<( pose const& x, pose const& y )
{
        if ( x.position == y.position )
                return x.orientation < y.orientation;
        return x.position < y.position;
}

/// compares poses on their position and orientation
constexpr bool operator==( pose const& x, pose const& y )
{
        return x.position == y.position && x.orientation == y.orientation;
}

/// negation of operator== between poses
constexpr bool operator!=( pose const& x, pose const& y )
{
        return !( x == y );
}

/// returns PoseDistance between provided poses
constexpr pose_distance distance_of( pose const& x, pose const& y )
{
        return {
            distance_of( x.position, y.position ),
            angle_shortest_path( x.orientation, y.orientation ) };
}

/// linear interpolation between base se and goal pose, with factor 0 'base' is returned, with
/// factor 1 'goal' is returned. With factor 0.5, pose between 'base' and 'goal' pose is returned
constexpr pose lin_interp( pose const& from, pose const& goal, float factor )
{
        return pose{
            lin_interp( from.position, goal.position, factor ),
            slerp( from.orientation, goal.orientation, factor ) };
}

inline std::vector< pose >
lineary_interpolate_path( std::vector< pose > const& ipath, float d_step, float a_step )
{
        std::vector< pose > res;
        if ( ipath.empty() )
                return res;
        for ( std::size_t const i : range( ipath.size() - 1 ) ) {
                pose const&       from      = ipath[i];
                pose const&       to        = ipath[i + 1];
                std::size_t const seg_steps = steps( distance_of( to, from ), d_step, a_step );
                for ( std::size_t const j : range( seg_steps ) )
                        res.push_back( lin_interp( from, to, float( j ) / float( seg_steps ) ) );
        }
        res.push_back( ipath.back() );
        return res;
}

/// Point A is rotated based on 'transformation' orientation and than moved based on
/// 'transformation' position
constexpr point< 3 > transform( point< 3 > const& a, pose const& transformation )
{
        return rotate( a, transformation.orientation ) + vector_cast( transformation.position );
}

constexpr vector< 3 > transform( vector< 3 > const& v, pose const& transformation )
{
        return rotate( v, transformation.orientation ) + vector_cast( transformation.position );
}

/// Pose X is rotated based on 'transformation' orientation and than moved based on 'transformation'
/// position
constexpr pose transform( pose const& x, pose const& transformation )
{
        return pose{
            transform( x.position, transformation ), transformation.orientation * x.orientation };
}

template < typename T >
constexpr auto transform( min_max< T > const& mm, pose const& transformation )
{
        return min_max{ transform( mm.min, transformation ), transform( mm.max, transformation ) };
}

constexpr pose inverse( pose const& x )
{
        return pose{ -x.position, inverse( x.orientation ) };
}

template < typename T >
constexpr auto inverse_transform( T const& item, pose const& transformation )
{
        return transform( item, inverse( transformation ) );
}

/// Pose X is rotated based on quaternion 'quad'
constexpr pose rotate( pose const& x, quaternion const& quat )
{
        return pose{ rotate( x.position, quat ), quat * x.orientation };
}

}  // namespace emlabcpp
