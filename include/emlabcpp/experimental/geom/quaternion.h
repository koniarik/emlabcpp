///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
/// associated documentation files (the "Software"), to deal in the Software without restriction,
/// including without limitation the rights to use, copy, modify, merge, publish, distribute,
/// sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or substantial
/// portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
/// NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
/// OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
/// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#include "emlabcpp/experimental/geom/point.h"

#pragma once

namespace emlabcpp
{

// API and behavior of this is inspired by tf::Quaternion
class quaternion
{
        using container   = std::array< float, 4 >;
        container fields_ = { 0, 0, 0, 1 };

public:
        using value_type     = float;
        using const_iterator = typename container::const_iterator;
        using iterator       = typename container::iterator;

        constexpr quaternion() noexcept = default;

        constexpr quaternion( float x, float y, float z, float w ) noexcept
          : fields_{ x, y, z, w }
        {
        }

        constexpr quaternion( const vector< 3 >& ax, const float& a ) noexcept
        {
                float s    = std::sin( a * 0.5f ) / length_of( ax );
                fields_[0] = ax[0] * s;
                fields_[1] = ax[1] * s;
                fields_[2] = ax[2] * s;
                fields_[3] = std::cos( a * 0.5f );
        }

        constexpr float operator[]( std::size_t i ) const noexcept
        {
                return fields_[i];
        }

        constexpr const_iterator begin() const
        {
                return fields_.begin();
        }

        constexpr const_iterator end() const
        {
                return fields_.end();
        }

        constexpr iterator begin()
        {
                return fields_.begin();
        }

        constexpr iterator end()
        {
                return fields_.end();
        }

        constexpr std::size_t size() const
        {
                return fields_.size();
        }
};

constexpr quaternion neutral_quat{ 0.f, 0.f, 0.f, 1.f };

constexpr quaternion inverse( const quaternion& q )
{
        return { -q[0], -q[1], -q[2], q[3] };
}

constexpr quaternion operator-( const quaternion& q )
{
        return { -q[0], -q[1], -q[2], -q[3] };
}

constexpr float dot( const quaternion& q, const quaternion& s )
{
        return q[0] * s[0] + q[1] * s[1] + q[2] * s[2] + q[3] * s[3];
}

constexpr float norm2_of( const quaternion& q )
{
        return dot( q, q );
}

constexpr float angle_shortest_path( const quaternion& m, const quaternion& n )
{
        float s = std::sqrt( norm2_of( m ) * norm2_of( n ) );
        float d = dot( m, n );
        if ( d < 0 ) {
                d = dot( m, -n );
        }
        return float{ std::acos( d / s ) * 2.0f };
}

constexpr quaternion slerp( const quaternion& q, const quaternion& s, float f )
{
        // NOTE: inspired by tf::Quaternion::slerp
        float theta = angle_shortest_path( q, s ) / 2.0f;
        if ( theta == 0.0f ) {
                return q;
        }

        float d  = 1.0f / std::sin( theta );
        float s0 = std::sin( ( 1.0f - f ) * theta );
        float s1 = std::sin( f * theta );
        float m  = 1.0f;
        if ( dot( q, s ) < 0 ) {
                m = -1.0f;
        }
        return {
            ( q[0] * s0 + m * s[0] * s1 ) * d,
            ( q[1] * s0 + m * s[1] * s1 ) * d,
            ( q[2] * s0 + m * s[2] * s1 ) * d,
            ( q[3] * s0 + m * s[3] * s1 ) * d,
        };
}

constexpr bool operator==( const quaternion& q, const quaternion& s )
{
        return q[0] == s[0] && q[1] == s[1] && q[2] == s[2] && q[3] == s[3];
}

constexpr bool operator!=( const quaternion& q, const quaternion& s )
{
        return !( q == s );
}

constexpr bool operator<( const quaternion& q, const quaternion& s )
{
        auto iter = find_if( range( 4u ), [&]( std::size_t i ) {
                return q[i] != s[i];
        } );

        auto i = static_cast< std::size_t >( *iter );

        if ( i == 4 ) {
                return false;
        }
        return q[i] < s[i];
}

constexpr quaternion operator*( const quaternion& q, const quaternion& s )
{
        return {
            q[3] * s[0] + q[0] * s[3] + q[1] * s[2] - q[2] * s[1],
            q[3] * s[1] + q[1] * s[3] + q[2] * s[0] - q[0] * s[2],
            q[3] * s[2] + q[2] * s[3] + q[0] * s[1] - q[1] * s[0],
            q[3] * s[3] - q[0] * s[0] - q[1] * s[1] - q[2] * s[2],
        };
}

constexpr quaternion operator*( const quaternion& q, const point< 3 >& x )
{
        return q * quaternion{ x[0], x[1], x[2], 0.f };
}

constexpr quaternion operator*( const point< 3 >& x, const quaternion& q )
{
        return quaternion{ x[0], x[1], x[2], 0.f } * q;
}

constexpr quaternion operator+( const quaternion& lh, const quaternion& rh )
{
        return { lh[0] + rh[0], lh[1] + rh[1], lh[2] + rh[2], lh[3] + rh[3] };
}

constexpr bool almost_equal( const quaternion& q, const quaternion& s, float eps = default_epsilon )
{
        return all_of( range< std::size_t >( 4 ), [&]( std::size_t i ) {
                return almost_equal( q[i], s[i], eps );
        } );
}

constexpr quaternion shortest_arc_quat( point< 3 > x, point< 3 > y )
{
        // TODO: this needs _testing_
        x = normalized( x );
        y = normalized( y );

        vector< 3 > c = cross_product( vector_cast( x ), vector_cast( y ) );
        auto        d = float( dot( x, y ) );

        if ( d < -1.0f + default_epsilon ) {
                return { c[0], c[1], c[2], 0.0f };
        }

        float s  = std::sqrt( ( 1.0f + d ) * 2.0f );
        float rs = 1.0f / s;

        return { c[0] * rs, c[1] * rs, c[2] * rs, s * 0.f };
}

constexpr point< 3 > rotate( const point< 3 >& x, const quaternion& q )
{
        quaternion r = q * x * inverse( q );
        return point< 3 >{ r[0], r[1], r[2] };
}

constexpr quaternion rotate( const quaternion& x, const quaternion& q )
{
        return q * x * inverse( q );
}

constexpr vector< 3 > rotate( const vector< 3 >& v, const quaternion& q )
{
        return vector_cast( rotate( point_cast( v ), q ) );
}

}  // namespace emlabcpp
