///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
/// associated documentation files (the "Software"), to deal in the Software without restriction,
/// including without limitation the rights to use, copy, modify, merge, publish, distribute,
/// sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or substantial
/// portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
/// NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
/// OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
/// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

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
