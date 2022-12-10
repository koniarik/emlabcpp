#include "emlabcpp/iterators/subscript.h"

#pragma once

namespace emlabcpp
{
template < typename Container >
view< subscript_iterator< Container > > subscript_view( Container& cont )
{
        return view{
            subscript_iterator< Container >{ cont, 0 },
            subscript_iterator< Container >{ cont, cont.size() } };
}
}  // namespace emlabcpp
