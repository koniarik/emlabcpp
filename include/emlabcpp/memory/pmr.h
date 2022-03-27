#include <memory_resource>

#pragma once

namespace emlabcpp
{

struct pmr_deleter
{
        std::pmr::memory_resource* res;

        template < typename T >
        void operator()( T* item )
        {
                std::destroy_at( item );
                res->deallocate( item, sizeof( T ), alignof( T ) );
        }
};

template < typename T >
using pmr_unique_ptr = std::unique_ptr< T, pmr_deleter >;

}  // namespace emlabcpp
