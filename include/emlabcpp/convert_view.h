
#include "emlabcpp/iterators/convert.h"

#pragma once

namespace emlabcpp
{

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
