///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
/// associated documentation files (the "Software"), to deal in the Software without restriction,
/// including without limitation the rights to use, copy, modify, merge, publish, distribute,
/// sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or substantial
/// portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
/// NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
/// OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
/// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#include "emlabcpp/view.h"

#include <iterator>

#pragma once

namespace emlabcpp
{
/// generic_iterator is a class using CRTP to ease implementation of custom iterators.
/// User of the class is required to inheric from generic_iterator and pass the inheriting class as
/// template argument Derived. The generic_iterator expectes existence of properly setup
/// std::iterator_traits<Derived> instance, which is used to decide types for various methods.
/// Given that generic_iterator is able to provide user with methods/functions that are based on the
/// Derived methods.
///
/// For example: It is necessary for Derived class to implement only operator==, given that
/// generic_iterator is able to provied !=
///
/// The Derived class is expected to provide these methods:
///  - reference operator*();
///  - const_reference operator*() const;
///  - Deriver& operator+=(difference_type);
///  - Deriver& operator-=(difference_type);
///  - bool operator<(const Derived & other);
///  - bool operator==(const Derived & other);
///  - difference_type operator-(const Derived& other);
///
/// Give nthese methods, the generic iterator provides additional methods thanks to CRTP mechanics:
///  - pointer operator->();
///  - const_pointer opetrator->();
///  - Derived& operator++();
///  - Derived& operator++(int);
///  - Derived& operator--();
///  - Derived& operator--(int);
///  - Derived operator+(difference_type);
///  - Derived operator-(difference_type);
///
/// Additional to that, following free functions are usable:
///  - bool operator>(const generic_iterator<Derived> &, const generic_iterator<Derived>&)
///  - bool operator<=(const generic_iterator<Derived> &, const generic_iterator<Derived>&)
///  - bool operator!=(const generic_iterator<Derived> &, const generic_iterator<Derived>&)
///
///
template < typename Derived >
class generic_iterator
{

        [[nodiscard]] constexpr Derived& impl()
        {
                return static_cast< Derived& >( *this );
        }
        [[nodiscard]] constexpr Derived const& impl() const
        {
                return static_cast< Derived const& >( *this );
        }

public:
        using value_type        = typename std::iterator_traits< Derived >::value_type;
        using reference         = typename std::iterator_traits< Derived >::reference;
        using const_reference   = const reference;
        using pointer           = typename std::iterator_traits< Derived >::pointer;
        using const_pointer     = typename std::iterator_traits< Derived >::const_pointer;
        using difference_type   = typename std::iterator_traits< Derived >::difference_type;
        using iterator_category = typename std::iterator_traits< Derived >::iterator_category;

        constexpr pointer operator->()
        {
                return &*impl();
        }

        constexpr const_pointer operator->() const
        {
                return &*impl();
        }

        constexpr Derived& operator++()
        {
                impl() += 1;
                return impl();
        }

        constexpr Derived operator++( const int )
        {
                auto copy = impl();
                impl() += 1;
                return copy;
        }

        constexpr Derived& operator--()
        {
                impl() -= 1;
                return impl();
        }

        constexpr Derived operator--( const int )
        {
                auto copy = impl();
                impl() -= 1;
                return copy;
        }

        constexpr auto operator<=>( const generic_iterator< Derived >& other ) const
        {
                return impl() <=> other.impl();
        }
        constexpr bool operator==( const generic_iterator< Derived >& other ) const
        {
                return impl() == other.impl();
        }

        constexpr Derived operator+( difference_type v ) const
        {
                auto copy = impl();
                copy += v;
                return copy;
        }

        constexpr Derived operator-( difference_type v ) const
        {
                auto copy = impl();
                copy -= v;
                return copy;
        }
};

}  // namespace emlabcpp
