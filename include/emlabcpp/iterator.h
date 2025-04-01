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

#include <iterator>

namespace emlabcpp
{

template < typename Derived >
concept nothrow_dereference = requires( Derived d ) {
        { *d } noexcept;
};

template < typename Derived >
concept nothrow_add_assign = requires( Derived d ) {
        { d += 1 } noexcept;
};

template < typename Derived >
concept nothrow_sub_assign = requires( Derived d ) {
        { d -= 1 } noexcept;
};

template < typename Derived >
concept nothrow_threeway_compare = requires( Derived d ) {
        { d <=> d } noexcept;
};

template < typename Derived >
concept nothrow_equality_compare = requires( Derived d ) {
        { d == d } noexcept;
};

/// generic_iterator simplifies custom iterator implementation using CRTP.
/// Users inherit from generic_iterator and pass their class as the Derived template argument.
/// It relies on std::iterator_traits<Derived> for type definitions and provides additional methods
/// based on those implemented in the Derived class.
///
/// The Derived class is expected to provide these methods:
///  - reference operator*();
///  - const_reference operator*() const;
///  - Derived& operator+=(difference_type);
///  - Derived& operator-=(difference_type);
///  - bool operator<(const Derived & other);
///  - bool operator==(const Derived & other);
///  - difference_type operator-(const Derived& other);
///
/// Given these methods, the generic iterator provides:
///  - pointer operator->();
///  - const_pointer operator->() const;
///  - Derived& operator++();
///  - Derived operator++(int);
///  - Derived& operator--();
///  - Derived operator--(int);
///  - Derived operator+(difference_type);
///  - Derived operator-(difference_type);
///
/// Additionally, the following free functions are usable:
///  - bool operator>(const generic_iterator<Derived> &, const generic_iterator<Derived>&);
///  - bool operator<=(const generic_iterator<Derived> &, const generic_iterator<Derived>&);
///  - bool operator!=(const generic_iterator<Derived> &, const generic_iterator<Derived>&);
///
///
template < typename Derived >
class generic_iterator
{

        [[nodiscard]] constexpr Derived& impl() noexcept
        {
                return static_cast< Derived& >( *this );
        }

        [[nodiscard]] constexpr Derived const& impl() const noexcept
        {
                return static_cast< Derived const& >( *this );
        }

        generic_iterator() noexcept = default;
        friend Derived;

public:
        using value_type        = typename std::iterator_traits< Derived >::value_type;
        using reference         = typename std::iterator_traits< Derived >::reference;
        using const_reference   = reference const;
        using pointer           = typename std::iterator_traits< Derived >::pointer;
        using const_pointer     = typename std::iterator_traits< Derived >::const_pointer;
        using difference_type   = typename std::iterator_traits< Derived >::difference_type;
        using iterator_category = typename std::iterator_traits< Derived >::iterator_category;

        constexpr pointer operator->() noexcept( nothrow_dereference< Derived > )
        {
                return &*impl();
        }

        constexpr const_pointer operator->() const noexcept( nothrow_dereference< Derived > )
        {
                return &*impl();
        }

        constexpr Derived& operator++() noexcept( nothrow_add_assign< Derived > )
        {
                impl() += 1;
                return impl();
        }

        constexpr Derived operator++( int const ) noexcept(
            nothrow_add_assign< Derived > && std::is_nothrow_copy_constructible_v< Derived > )
        {
                auto copy = impl();
                impl() += 1;
                return copy;
        }

        constexpr Derived& operator--() noexcept( nothrow_sub_assign< Derived > )
        {
                impl() -= 1;
                return impl();
        }

        constexpr Derived operator--( int const ) noexcept(
            nothrow_sub_assign< Derived > && std::is_nothrow_copy_constructible_v< Derived > )
        {
                auto copy = impl();
                impl() -= 1;
                return copy;
        }

        constexpr auto operator<=>( generic_iterator< Derived > const& other ) const
            noexcept( nothrow_threeway_compare< Derived > )
        {
                return impl() <=> other.impl();
        }

        constexpr bool operator==( generic_iterator< Derived > const& other ) const
            noexcept( nothrow_equality_compare< Derived > )
        {
                return impl() == other.impl();
        }

        constexpr Derived operator+( difference_type v ) const noexcept(
            nothrow_add_assign< Derived > && std::is_nothrow_copy_constructible_v< Derived > )
        {
                auto copy = impl();
                copy += v;
                return copy;
        }

        constexpr Derived operator-( difference_type v ) const noexcept(
            nothrow_sub_assign< Derived > && std::is_nothrow_copy_constructible_v< Derived > )
        {
                auto copy = impl();
                copy -= v;
                return copy;
        }
};

}  // namespace emlabcpp
