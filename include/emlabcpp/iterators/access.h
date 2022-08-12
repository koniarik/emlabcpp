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
#include "emlabcpp/iterator.h"

#pragma once

namespace emlabcpp
{
template < typename, typename >
class access_iterator;
}

template < typename Iterator, typename AccessCallable >
struct std::iterator_traits< emlabcpp::access_iterator< Iterator, AccessCallable > >
{
        using value_type      = std::remove_reference_t< decltype( std::declval< AccessCallable >()(
            *std::declval< Iterator >() ) ) >;
        using difference_type = std::ptrdiff_t;
        using pointer         = value_type*;
        using const_pointer   = const value_type*;
        using reference       = value_type&;
        using iterator_category = std::random_access_iterator_tag;
};

namespace emlabcpp
{

/// access_iterator provides access to a reference of value stored in the Iterator.
/// The access is provided via the AccessCallable provided to the iterator.
///
/// Gives you abillity to iterate over dataset, while accessing only part of each item.
///
template < typename Iterator, typename AccessCallable >
class access_iterator : public generic_iterator< access_iterator< Iterator, AccessCallable > >
{
        Iterator       current_;
        AccessCallable fun_;

public:
        using value_type = typename std::iterator_traits<
            access_iterator< Iterator, AccessCallable > >::value_type;

        constexpr access_iterator( Iterator current, AccessCallable f )
          : current_( std::move( current ) )
          , fun_( std::move( f ) )
        {
        }

        constexpr value_type& operator*()
        {
                return fun_( *current_ );
        }
        constexpr const value_type& operator*() const
        {
                return fun_( *current_ );
        }

        constexpr access_iterator& operator+=( std::ptrdiff_t offset )
        {
                std::advance( current_, offset );
                return *this;
        }
        constexpr access_iterator& operator-=( std::ptrdiff_t offset )
        {
                std::advance( current_, -offset );
                return *this;
        }

        constexpr auto operator<=>( const access_iterator& other ) const
        {
                return current_ <=> other.current_;
        }
        constexpr bool operator==( const access_iterator& other ) const
        {
                return current_ == other.current_;
        }

        constexpr std::ptrdiff_t operator-( const access_iterator& other )
        {
                return current_ - other.current_;
        }
};

/// Creates view ver container cont with AccessCallable f.
/// Beware that this produces two copies of f!
template < typename Container, typename AccessCallable >
view< access_iterator< iterator_of_t< Container >, AccessCallable > >
access_view( Container&& cont, AccessCallable&& f )
{
        return view{
            access_iterator< iterator_of_t< Container >, AccessCallable >{ cont.begin(), f },
            access_iterator< iterator_of_t< Container >, AccessCallable >{ cont.end(), f } };
}

}  // namespace emlabcpp
