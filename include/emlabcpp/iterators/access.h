#include "emlabcpp/iterator.h"

#pragma once

namespace emlabcpp
{
template < typename, typename >
class access_iterator;
}

template < typename Iterator, typename AccessFunction >
struct std::iterator_traits< emlabcpp::access_iterator< Iterator, AccessFunction > >
{
        using value_type      = std::remove_reference_t< decltype( std::declval< AccessFunction >()(
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
/// The access is provided via the AccessFunction provided to the iterator.
///
/// Gives you abillity to iterate over dataset, while accessing only part of each item.
///
template < typename Iterator, typename AccessFunction >
class access_iterator : public generic_iterator< access_iterator< Iterator, AccessFunction > >
{
        Iterator       current_;
        AccessFunction fun_;

public:
        using value_type = typename std::iterator_traits<
            access_iterator< Iterator, AccessFunction > >::value_type;

        constexpr access_iterator( Iterator current, AccessFunction f )
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

/// Creates view ver container cont with AccessFunction f.
/// Beware that this produces two copies of f!
template < typename Container, typename AccessFunction >
view< access_iterator< iterator_of_t< Container >, AccessFunction > >
access_view( Container&& cont, AccessFunction&& f )
{
        return view{
            access_iterator< iterator_of_t< Container >, AccessFunction >{ cont.begin(), f },
            access_iterator< iterator_of_t< Container >, AccessFunction >{ cont.end(), f } };
}

}  /// namespace emlabcpp
