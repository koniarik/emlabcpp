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

#include "./point.h"

namespace emlabcpp
{

/// API and behavior of this is inspired by tf::Quaternion
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

        constexpr quaternion( vector< 3 > const& ax, float const& a ) noexcept
        {
                float const s = std::sin( a * 0.5f ) / length_of( ax );
                fields_[0]    = ax[0] * s;
                fields_[1]    = ax[1] * s;
                fields_[2]    = ax[2] * s;
                fields_[3]    = std::cos( a * 0.5f );
        }

        [[nodiscard]] constexpr float operator[]( std::size_t i ) const noexcept
        {
                return fields_[i];
        }

        [[nodiscard]] constexpr const_iterator begin() const
        {
                return fields_.begin();
        }

        [[nodiscard]] constexpr const_iterator end() const
        {
                return fields_.end();
        }

        [[nodiscard]] constexpr iterator begin()
        {
                return fields_.begin();
        }

        [[nodiscard]] constexpr iterator end()
        {
                return fields_.end();
        }

        [[nodiscard]] constexpr std::size_t size() const
        {
                return fields_.size();
        }
};

constexpr quaternion neutral_quat{ 0.f, 0.f, 0.f, 1.f };

constexpr quaternion inverse( quaternion const& q )
{
        return { -q[0], -q[1], -q[2], q[3] };
}

constexpr quaternion operator-( quaternion const& q )
{
        return { -q[0], -q[1], -q[2], -q[3] };
}

constexpr float dot( quaternion const& q, quaternion const& s )
{
        return q[0] * s[0] + q[1] * s[1] + q[2] * s[2] + q[3] * s[3];
}

constexpr float norm2_of( quaternion const& q )
{
        return dot( q, q );
}

constexpr float angle_shortest_path( quaternion const& m, quaternion const& n )
{
        float const s = std::sqrt( norm2_of( m ) * norm2_of( n ) );
        float       d = dot( m, n );
        if ( d < 0 )
                d = dot( m, -n );
        return float{ std::acos( d / s ) * 2.0f };
}

constexpr quaternion slerp( quaternion const& q, quaternion const& s, float f )
{
        // NOTE: inspired by tf::Quaternion::slerp
        float const theta = angle_shortest_path( q, s ) / 2.0f;
        if ( theta == 0.0f )
                return q;

        float const d  = 1.0f / std::sin( theta );
        float const s0 = std::sin( ( 1.0f - f ) * theta );
        float const s1 = std::sin( f * theta );
        float       m  = 1.0f;
        if ( dot( q, s ) < 0 )
                m = -1.0f;
        return {
            ( q[0] * s0 + m * s[0] * s1 ) * d,
            ( q[1] * s0 + m * s[1] * s1 ) * d,
            ( q[2] * s0 + m * s[2] * s1 ) * d,
            ( q[3] * s0 + m * s[3] * s1 ) * d,
        };
}

constexpr bool operator==( quaternion const& q, quaternion const& s )
{
        return q[0] == s[0] && q[1] == s[1] && q[2] == s[2] && q[3] == s[3];
}

constexpr bool operator!=( quaternion const& q, quaternion const& s )
{
        return !( q == s );
}

constexpr bool operator<( quaternion const& q, quaternion const& s )
{
        auto iter = find_if( range( 4u ), [&]( std::size_t i ) {
                return q[i] != s[i];
        } );

        auto i = static_cast< std::size_t >( *iter );

        if ( i == 4 )
                return false;
        return q[i] < s[i];
}

constexpr quaternion operator*( quaternion const& q, quaternion const& s )
{
        return {
            q[3] * s[0] + q[0] * s[3] + q[1] * s[2] - q[2] * s[1],
            q[3] * s[1] + q[1] * s[3] + q[2] * s[0] - q[0] * s[2],
            q[3] * s[2] + q[2] * s[3] + q[0] * s[1] - q[1] * s[0],
            q[3] * s[3] - q[0] * s[0] - q[1] * s[1] - q[2] * s[2],
        };
}

constexpr quaternion operator*( quaternion const& q, point< 3 > const& x )
{
        return q * quaternion{ x[0], x[1], x[2], 0.f };
}

constexpr quaternion operator*( point< 3 > const& x, quaternion const& q )
{
        return quaternion{ x[0], x[1], x[2], 0.f } * q;
}

constexpr quaternion operator+( quaternion const& lh, quaternion const& rh )
{
        return { lh[0] + rh[0], lh[1] + rh[1], lh[2] + rh[2], lh[3] + rh[3] };
}

constexpr bool almost_equal( quaternion const& q, quaternion const& s, float eps = default_epsilon )
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

        if ( d < -1.0f + default_epsilon )
                return { c[0], c[1], c[2], 0.0f };

        float const s  = std::sqrt( ( 1.0f + d ) * 2.0f );
        float const rs = 1.0f / s;

        return { c[0] * rs, c[1] * rs, c[2] * rs, s * 0.f };
}

constexpr point< 3 > rotate( point< 3 > const& x, quaternion const& q )
{
        quaternion const r = q * x * inverse( q );
        return point< 3 >{ r[0], r[1], r[2] };
}

constexpr quaternion rotate( quaternion const& x, quaternion const& q )
{
        return q * x * inverse( q );
}

constexpr vector< 3 > rotate( vector< 3 > const& v, quaternion const& q )
{
        return vector_cast( rotate( point_cast( v ), q ) );
}

}  // namespace emlabcpp
