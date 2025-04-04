/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
///

#pragma once

#include "./memory_resource.h"
#include "./throw_bad_alloc.h"

#include <functional>

namespace emlabcpp::pmr
{

template < typename T >
class allocator
{
public:
        using value_type = T;

        template < typename U >
        allocator( allocator< U > const& other )
          : resource_( other.resource_ )
        {
        }

        allocator( pmr::memory_resource& src )
          : resource_( src )
        {
        }

        T* allocate( std::size_t const n )
        {
                void* const res = resource_.get().allocate( n * sizeof( T ), alignof( T ) );
                if ( res == nullptr )
                        throw_bad_alloc();
                return reinterpret_cast< T* >( res );
        }

        void deallocate( T* const p, std::size_t const n ) const
        {
                result const res = resource_.get().deallocate(
                    reinterpret_cast< void* >( p ), n * sizeof( T ), alignof( T ) );
                if ( res == result::ERROR )
                        throw_bad_alloc();
        }

        friend constexpr bool operator==( allocator const& lh, allocator const& rh )
        {
                return lh.resource_.get().is_equal( rh.resource_.get() );
        }

        pmr::memory_resource& get_resource()
        {
                return resource_;
        }

        template < typename U >
        friend class allocator;

private:
        std::reference_wrapper< pmr::memory_resource > resource_;
};

}  // namespace emlabcpp::pmr
