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
#pragma once

#include <concepts>

namespace emlabcpp
{

template < typename Derived, typename Enum, Enum SuccessValue >
struct [[nodiscard]] status
{
        using enum_type = Enum;

        constexpr status( enum_type s ) noexcept
          : s_( s )
        {
        }

        constexpr status( status const& ) noexcept            = default;
        constexpr status( status&& ) noexcept                 = default;
        constexpr status& operator=( status const& ) noexcept = default;
        constexpr status& operator=( status&& ) noexcept      = default;

        [[nodiscard]] constexpr enum_type value() const noexcept
        {
                return s_;
        }

        constexpr friend auto
        operator<=>( status const& lhs, status const& rhs ) noexcept = default;

private:
        enum_type s_;
};

}  // namespace emlabcpp
