#pragma once

#include "emlabcpp/zip.h"

namespace emlabcpp
{

template < typename Container >
auto enumerate( Container&& cont )
{
        return zip( range( cont.size() ), cont );
}

}  // namespace emlabcpp
