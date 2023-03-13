
#include "emlabcpp/experimental/geom/point.h"
#include "emlabcpp/experimental/geom/pose.h"
#include "emlabcpp/experimental/geom/quaternion.h"
#include "emlabcpp/experimental/geom/simplex.h"

#ifdef EMLABCPP_USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

#pragma once

namespace emlabcpp
{
#ifdef EMLABCPP_USE_NLOHMANN_JSON

template < std::size_t N >
inline void from_json( const nlohmann::json& j, point< N >& p )
{
        using container = typename point< N >::container;

        p = point< N >( j.get< container >() );
}

template < std::size_t N >
inline void to_json( nlohmann::json& j, const point< N >& p )
{
        j = view{ p };
}

inline void to_json( nlohmann::json& j, const quaternion& quat )
{
        j = { quat[0], quat[1], quat[2], quat[3] };
}

inline void from_json( const nlohmann::json& j, quaternion& quat )
{
        quat = quaternion{
            j.at( 0 ).get< float >(),
            j.at( 1 ).get< float >(),
            j.at( 2 ).get< float >(),
            j.at( 3 ).get< float >() };
}

inline void to_json( nlohmann::json& j, const pose& p )
{
        j["position"]    = p.position;
        j["orientation"] = p.orientation;
}

inline void from_json( const nlohmann::json& j, pose& p )
{
        p = pose{
            j.at( "position" ).get< point< 3 > >(), j.at( "orientation" ).get< quaternion >() };
}

template < typename Item, std::size_t N >
inline void to_json( nlohmann::json& j, const simplex< Item, N >& sim )
{
        std::copy( sim.begin(), sim.end(), std::back_inserter( j ) );
}

template < typename Item, std::size_t N >
inline void from_json( const nlohmann::json& j, simplex< Item, N >& sim )
{
        sim = simplex< Item, N >{ j.get< std::array< Item, N + 1 > >() };
}

#endif

}  // namespace emlabcpp
