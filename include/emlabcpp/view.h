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

#include <span>
#include "emlabcpp/types.h"

#pragma once

namespace emlabcpp
{

/// Generic class to represent view of some container.
///
/// The view stores iterators to the input container and acts as container.
///
/// Note: is_view_v<T> can be used to detect if T is view<I>
///
template < typename Iterator >
class view
{
        Iterator begin_;
        Iterator end_;

public:
        /// standard public usings for container
        using value_type       = typename std::iterator_traits< Iterator >::value_type;
        using reverse_iterator = std::reverse_iterator< Iterator >;
        using iterator         = Iterator;
        using difference_type  = typename std::iterator_traits< Iterator >::difference_type;
        using size_type        = std::size_t;

        constexpr view() = default;

        /// constructor from Container, uses begin/end of the container
        template < range_container Cont >
        requires( std::convertible_to< iterator_of_t< Cont >, iterator > )
        constexpr view( Cont& cont )
          : begin_( std::begin( cont ) )
          , end_( std::end( cont ) )
        {
        }

        /// constructor from Container, uses begin/end of the container
        template < range_container Cont >
        requires( std::convertible_to< iterator_of_t< Cont >, iterator > )
        constexpr view( const Cont& cont )
          : begin_( std::begin( cont ) )
          , end_( std::end( cont ) )
        {
        }

        /// constructor from the iterators that should internally be stored
        constexpr view( Iterator begin, Iterator end )
          : begin_( std::move( begin ) )
          , end_( std::move( end ) )
        {
        }

        template < std::convertible_to< Iterator > OtherIterator >
        constexpr view( view< OtherIterator > other )
          : begin_( other.begin() )
          , end_( other.end() )
        {
        }

        /// Start of the dataset iterator
        [[nodiscard]] constexpr Iterator begin() const
        {
                return begin_;
        }

        /// Past the end iterator
        [[nodiscard]] constexpr Iterator end() const
        {
                return end_;
        }

        /// Access to i-th element in the range, expects Iterator::operator+
        [[nodiscard]] constexpr decltype( auto ) operator[]( const size_type i ) const
        {
                return *( begin_ + static_cast< difference_type >( i ) );
        }

        /// Returns iterator to the last element that goes in reverse
        [[nodiscard]] constexpr reverse_iterator rbegin() const
        {
                return reverse_iterator{ end_ };
        }

        /// Returns iterator to the element before first element, that can go in
        /// reverse
        [[nodiscard]] constexpr reverse_iterator rend() const
        {
                return reverse_iterator{ begin_ };
        }

        /// Size of the view over dataset uses std::distance() to tell the size
        [[nodiscard]] constexpr size_type size() const
        {
                return static_cast< std::size_t >( std::distance( begin(), end() ) );
        }

        /// View is empty if both iterators are equal
        [[nodiscard]] constexpr bool empty() const
        {
                return begin() == end();
        }

        /// Returns first value of the range
        [[nodiscard]] constexpr const value_type& front() const
        {
                return *begin_;
        }

        /// Returns last value of the range
        [[nodiscard]] constexpr const value_type& back() const
        {
                return *std::prev( end_ );
        }

        operator std::span< value_type >()
        {
                return std::span{ begin(), end() };
        }
};

template < typename IteratorLh, typename IteratorRh >
constexpr bool operator==( const view< IteratorLh >& lh, const view< IteratorRh >& rh )
{
        if ( lh.size() != rh.size() ) {
                return false;
        }

        IteratorLh lhiter = lh.begin();
        IteratorRh rhiter = rh.begin();

        for ( ; lhiter != lh.end(); ++lhiter, ++rhiter ) {
                if ( *lhiter != *rhiter ) {
                        return false;
                }
        }
        return true;
}

template < typename IteratorLh, typename IteratorRh >
constexpr bool operator!=( const view< IteratorLh >& lh, const view< IteratorRh >& rh )
{
        return !( lh == rh );
}

/// The container deduction guide uses iterator_of_t
template < range_container Container >
view( Container& cont ) -> view< iterator_of_t< Container > >;

/// Support for our deduction guide to types - is_view_v
template < typename Iter >
struct impl::is_view< view< Iter > > : std::true_type
{
};

/// Creates view over 'n' items of dataset starting at 'begin'
/// This does not check validity of the range!
template < typename Iter >
constexpr view< Iter > view_n( Iter begin, const std::size_t n )
{
        auto end = std::next(
            begin, static_cast< typename std::iterator_traits< Iter >::difference_type >( n ) );
        return view< Iter >{ std::move( begin ), end };
}

template < typename Container >
constexpr auto data_view( Container& cont )
{
        return view_n( std::data( cont ), std::size( cont ) );
}

/// Creates the view over over Container, where we ignore first r*size/2 items
/// and last r*size/2 items. This can be used to get the dataset without
/// first/last 5% for example, by using r=0.1
template < range_container Container >
constexpr view< iterator_of_t< Container > > trim_view( Container& cont, const float r )
{
        const std::size_t step = std::size( cont ) * ( 1.f - r ) / 2.f;
        return { std::begin( cont ) + step, std::end( cont ) - step };
}

/// Returns view to the Container in reverse order.
constexpr auto reversed( referenceable_container auto& container )
    -> view< decltype( std::rbegin( container ) ) >
{
        return { std::rbegin( container ), std::rend( container ) };
}

template < typename Iterator >
void string_serialize_view( auto&& w, const view< Iterator >& output )
{
        using value_type = typename std::iterator_traits< Iterator >::value_type;
        bool first       = true;
        for ( const value_type& item : output ) {
                if ( !first ) {
                        w( ',' );
                }
                w( item );
                first = false;
        }
}

#ifdef EMLABCPP_USE_OSTREAM
template < typename Iterator >
std::ostream& operator<<( std::ostream& os, const view< Iterator >& iter )
{
        string_serialize_view(
            [&]( const auto& item ) {
                    os << item;
            },
            iter );
        return os;
}
#endif

}  // namespace emlabcpp
