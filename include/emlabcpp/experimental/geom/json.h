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
#include "./pose.h"
#include "./quaternion.h"
#include "./simplex.h"

#ifdef EMLABCPP_USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

namespace emlabcpp
{
#ifdef EMLABCPP_USE_NLOHMANN_JSON

template < std::size_t N >
inline void from_json( nlohmann::json const& j, point< N >& p )
{
        using container = typename point< N >::container;

        p = point< N >( j.get< container >() );
}

template < std::size_t N >
inline void to_json( nlohmann::json& j, point< N > const& p )
{
        j = view{ p };
}

inline void to_json( nlohmann::json& j, quaternion const& quat )
{
        j = { quat[0], quat[1], quat[2], quat[3] };
}

inline void from_json( nlohmann::json const& j, quaternion& quat )
{
        quat = quaternion{
            j.at( 0 ).get< float >(),
            j.at( 1 ).get< float >(),
            j.at( 2 ).get< float >(),
            j.at( 3 ).get< float >() };
}

inline void to_json( nlohmann::json& j, pose const& p )
{
        j["position"]    = p.position;
        j["orientation"] = p.orientation;
}

inline void from_json( nlohmann::json const& j, pose& p )
{
        p = pose{
            j.at( "position" ).get< point< 3 > >(), j.at( "orientation" ).get< quaternion >() };
}

template < typename Item, std::size_t N >
inline void to_json( nlohmann::json& j, simplex< Item, N > const& sim )
{
        std::copy( sim.begin(), sim.end(), std::back_inserter( j ) );
}

template < typename Item, std::size_t N >
inline void from_json( nlohmann::json const& j, simplex< Item, N >& sim )
{
        sim = simplex< Item, N >{ j.get< std::array< Item, N + 1 > >() };
}

#endif

}  // namespace emlabcpp
