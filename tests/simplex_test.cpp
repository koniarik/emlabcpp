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

#include "emlabcpp/experimental/geom/simplex.h"

#include "emlabcpp/experimental/geom/json.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

TEST( SimplexTest, volume2d )
{
        const std::array< point< 2 >, 3 > data{
            point< 2 >( 0, 1 ), point< 2 >( 1, 0 ), point< 2 >( 0, 0 ) };

        const simplex< point< 2 >, 2 > sim( data );

        ASSERT_NEAR( volume_of( sim ), 0.5f, 0.001f );
}

TEST( SimplexTest, volume3d )
{
        const std::array< point< 3 >, 4 > data{
            point< 3 >( 0, 0, 1 ),
            point< 3 >( 1, 0, 0 ),
            point< 3 >( 0, 0, 0 ),
            point< 3 >{ 0, 1, 0 } };

        const simplex< point< 3 >, 3 > sim( data );

        ASSERT_NEAR( volume_of( sim ), 0.166667f, 0.00001f );
}

TEST( SimplexTest, json )
{
        const std::array< point< 3 >, 4 > data{
            point< 3 >( 0, 0, 1 ),
            point< 3 >( 1, 0, 0 ),
            point< 3 >( 0, 0, 0 ),
            point< 3 >{ 0, 1, 0 } };

        using simplex_type = simplex< point< 3 >, 3 >;

        const simplex_type sim( data );

        const nlohmann::json simplex_j = nlohmann::json( sim );
        ASSERT_EQ( simplex_j.get< simplex_type >(), sim );
}

}  // namespace emlabcpp
