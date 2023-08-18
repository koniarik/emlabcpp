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

#include "emlabcpp/concepts.h"

#include <gtest/gtest.h>
#include <ostream>

namespace emlabcpp
{

static_assert( arithmetic_like< float > );
static_assert( arithmetic_like< int > );
static_assert( !arithmetic_like< std::string > );

static_assert( gettable_container< std::array< int, 4 > > );
static_assert( gettable_container< std::tuple< int, float > > );
static_assert( !gettable_container< std::vector< int > > );

static_assert( range_container< std::array< int, 4 > > );
static_assert( !range_container< std::tuple< int, float > > );
static_assert( range_container< std::vector< int > > );
static_assert( range_container< char[42] > );

static_assert( static_sized< std::array< int, 4 > > );
static_assert( static_sized< std::tuple< int, float > > );
static_assert( !static_sized< std::vector< int > > );

static_assert( ostreamlike< std::ostream > );
static_assert( !ostreamlike< const char[10] > );

static_assert( alternative_of< int, std::variant< int, float > > );
static_assert( !alternative_of< std::string, std::variant< int, float > > );

}  // namespace emlabcpp
