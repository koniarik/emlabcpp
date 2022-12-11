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

namespace emlabcpp::iterators
{

template < typename >
class numeric_iterator;

}

template < typename T >
struct std::iterator_traits< emlabcpp::iterators::numeric_iterator< T > >
{
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = T*;
        using const_pointer     = const T*;
        using reference         = T&;
        using iterator_category = std::random_access_iterator_tag;
};

namespace emlabcpp::iterators
{

/// numeric iterator - iterator over numbers (which are calculated on the fly)
/// Value of type T is stored internally and incremented as the iterator is moved forward/backward
///
/// T has to be any type for which operators +=, <, ++ and conversion to std::ptrdiff_t are defined.
template < typename T >
class numeric_iterator : public generic_iterator< numeric_iterator< T > >
{
        T val_;

public:
        /// Initializes iterator to value val
        explicit constexpr numeric_iterator( T val )
          : val_( std::move( val ) )
        {
        }

        constexpr T& operator*()
        {
                return val_;
        }
        constexpr const T& operator*() const
        {
                return val_;
        }

        constexpr numeric_iterator& operator+=( std::ptrdiff_t offset )
        {
                val_ += static_cast< T >( offset );
                return *this;
        }
        constexpr numeric_iterator& operator-=( std::ptrdiff_t offset )
        {
                val_ -= static_cast< T >( offset );
                return *this;
        }

        constexpr auto operator<=>( const numeric_iterator< T >& other ) const
        {
                return val_ <=> other.val_;
        }
        constexpr bool operator==( const numeric_iterator< T >& other ) const
        {
                return val_ == other.val_;
        }

        constexpr std::ptrdiff_t operator-( const numeric_iterator& other ) const
        {
                return static_cast< std::ptrdiff_t >( val_ ) -
                       static_cast< std::ptrdiff_t >( other.val_ );
        }
};

}  // namespace emlabcpp::iterators
