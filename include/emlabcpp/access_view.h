
#include "emlabcpp/iterators/access.h"

#pragma once

namespace emlabcpp
{

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
