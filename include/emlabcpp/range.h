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

#include "./iterators/numeric.h"
#include "./view.h"

namespace emlabcpp
{

/// Builds numeric view over interval [from, to)
template < typename Numeric >
constexpr view< iterators::numeric_iterator< Numeric > > range( Numeric from, Numeric to )
{
        return {
            iterators::numeric_iterator< Numeric >{ from },
            iterators::numeric_iterator< Numeric >{ to } };
}

/// Builds numeric view over interval [0, to)
template < typename Numeric >
constexpr view< iterators::numeric_iterator< Numeric > > range( Numeric to )
{
        return range< Numeric >( 0, to );
}

}  // namespace emlabcpp
