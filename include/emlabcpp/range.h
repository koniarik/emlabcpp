
#include "emlabcpp/iterators/numeric.h"

#pragma once

namespace emlabcpp
{

/// Builds numeric view over interval [from, to)
template < typename Numeric >
constexpr view< iterators::numeric_iterator< Numeric > > range( Numeric from, Numeric to )
{
        return {
            iterators::numeric_iterator< Numeric >{ from },
            iterators::numeric_iterator< Numeric >{ to } };
}

/// Builds numeric view over interval [0, to)
template < typename Numeric >
constexpr view< iterators::numeric_iterator< Numeric > > range( Numeric to )
{
        return range< Numeric >( 0, to );
}

}  // namespace emlabcpp
