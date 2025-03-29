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

#include "./iterators/convert.h"

namespace emlabcpp
{

template < typename T, typename Container, typename Iterator = iterator_of_t< Container > >
view< iterators::convert_iterator< T, Iterator > > convert_view( Container&& cont )
{
        return view{
            iterators::convert_iterator< T, Iterator >{ cont.begin() },
            iterators::convert_iterator< T, Iterator >{ cont.end() } };
}

template < typename T, typename Iterator >
constexpr view< iterators::convert_iterator< T, Iterator > >
convert_view_n( Iterator begin, std::size_t n )
{
        return view_n( iterators::convert_iterator< T, Iterator >{ begin }, n );
}

}  // namespace emlabcpp
