// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//
//  Copyright © 2022 Jan Veverak Koniarik
//  This file is part of project: emlabcpp
//
#include <utility>

#pragma once

namespace emlabcpp
{

/// Defer is an object that receives and stores callable, which is called on it's destruction. The
/// mechanism should be used to setup callbacks executed on end of scope.
///
/// For example:
///
/// void foo(){
///     defer d = []{
///             std::cout << "tree" << std::endl;
///     };
/// }
///
/// Prints "tree" once foo() finishes it's execution
template < typename Callable >
class defer
{
public:
        defer( Callable f )
          : fun_( std::move( f ) )
        {
        }

        defer( const defer& )            = default;
        defer( defer&& )                 = default;
        defer& operator=( const defer& ) = default;
        defer& operator=( defer&& )      = default;

        ~defer()
        {
                fun_();
        }

private:
        Callable fun_;
};

}  // namespace emlabcpp
