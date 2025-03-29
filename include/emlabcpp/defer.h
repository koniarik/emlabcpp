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

#include <utility>

namespace emlabcpp
{

/// The `defer` class stores a callable object and ensures it is executed when the `defer` object
/// goes out of scope. This is useful for setting up cleanup or callback actions to be executed
/// at the end of a scope.
///
/// Example:
///
/// void foo() {
///     defer d = [] {
///         std::cout << "tree" << std::endl;
///     };
/// }
///
/// This will print "tree" when `foo()` finishes execution.
template < typename Callable >
class defer
{
public:
        defer( Callable f )
          : fun_( std::move( f ) )
        {
        }

        defer( defer const& )                = default;
        defer( defer&& ) noexcept            = default;
        defer& operator=( defer const& )     = default;
        defer& operator=( defer&& ) noexcept = default;

        ~defer()
        {
                fun_();
        }

private:
        Callable fun_;
};

}  // namespace emlabcpp
