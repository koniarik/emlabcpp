#include <utility>

#pragma once

namespace emlabcpp
{

template < typename Function >
class defer
{
        Function fun_;

public:
        defer( Function f )
          : fun_( std::move( f ) )
        {
        }

        ~defer()
        {
                fun_();
        }
};

}  /// namespace emlabcpp
