#include "emlabcpp/iterator.h"

#pragma once

namespace emlabcpp
{
template < typename, typename >
class convert_iterator;
}

template < typename T, typename Iterator >
struct std::iterator_traits< emlabcpp::convert_iterator< T, Iterator > >
{
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = value_type;
        using const_pointer     = value_type;
        using reference         = value_type;
        using iterator_category = std::random_access_iterator_tag;
        // TODO: ^^ this is failure, should be decided based on Iterator
};

namespace emlabcpp
{
template < typename T, typename Iterator >
class convert_iterator : public generic_iterator< convert_iterator< T, Iterator > >
{
        Iterator iter_;

public:
        explicit convert_iterator( Iterator iter )
          : iter_( std::move( iter ) )
        {
        }

        T operator*()
        {
                return T{ *iter_ };
        }
        T operator*() const
        {
                return T{ *iter_ };
        }

        convert_iterator& operator+=( std::ptrdiff_t offset )
        {
                std::advance( iter_, offset );
                return *this;
        }

        convert_iterator& operator-=( std::ptrdiff_t offset )
        {
                std::advance( iter_, -offset );
                return *this;
        }

        auto operator<=>( const convert_iterator& other ) const
        {
                return iter_ <=> other.iter_;
        }

        bool operator==( const convert_iterator& other ) const
        {
                return iter_ == other.iter_;
        }

        std::ptrdiff_t operator-( const convert_iterator& other )
        {
                return iter_ - other.iter_;
        }
};

template < typename T, typename Container, typename Iterator = iterator_of_t< Container > >
view< convert_iterator< T, Iterator > > convert_view( Container&& cont )
{
        return view{
            convert_iterator< T, Iterator >{ cont.begin() },
            convert_iterator< T, Iterator >{ cont.end() } };
}

template < typename T, typename Iterator >
constexpr view< convert_iterator< T, Iterator > > convert_view_n( Iterator begin, std::size_t n )
{
        return view_n( convert_iterator< T, Iterator >{ begin }, n );
}

}  // namespace emlabcpp
