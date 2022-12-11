#include "emlabcpp/pmr/allocator.h"

#include <deque>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <vector>

#pragma once

namespace emlabcpp::pmr
{

struct deleter
{
        std::reference_wrapper< pmr::memory_resource > res;

        template < typename T >
        void operator()( T* item ) const
        {
                std::destroy_at( item );
                res.get().deallocate( item, sizeof( T ), alignof( T ) );
        }
};

template < typename T >
using unique_ptr = std::unique_ptr< T, deleter >;

template < typename T >
using vector = std::vector< T, allocator< T > >;
template < typename T >
using list = std::list< T, allocator< T > >;
template < typename T >
using set = std::set< T, allocator< T > >;
template < typename T >
using deque = std::deque< T, allocator< T > >;
template < typename Key, typename T >
using map = std::map< Key, T, std::less< Key >, allocator< std::pair< const Key, T > > >;

}  // namespace emlabcpp::pmr
