///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
/// and associated documentation files (the "Software"), to deal in the Software without
/// restriction, including without limitation the rights to use, copy, modify, merge, publish,
/// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or
/// substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
/// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
/// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#include "emlabcpp/allocator/util.h"

#include <concepts>

#pragma once

namespace emlabcpp
{

template < typename T, std::size_t N >
class derived_storage
{
public:
        static_assert(
            std::has_virtual_destructor_v< T >,
            "Type stored in derived_storage has to have a virtual destructor" );

        using storage_type = std::byte[N];

        using base_type = T;

        derived_storage()
          : ptr_( nullptr )
        {
        }

        template < std::derived_from< T > U >
        derived_storage( U&& item )
          : ptr_( &storage_ )
        {
                static_assert( is_storable_with_align< U >( N ) );
                ptr_ = align( ptr_, alignof( U ) );
                std::construct_at( ptr_, std::forward< U >( item ) );
        }

        derived_storage( const derived_storage& ) = delete;
        derived_storage( derived_storage&& )      = delete;

        derived_storage& operator=( const derived_storage& ) = delete;
        derived_storage& operator=( derived_storage&& )      = delete;

        T& operator*()
        {
                return *ptr_;
        }

        const T& operator*() const
        {
                return *ptr_;
        }

        T* operator->()
        {
                return ptr_;
        }

        const T* operator->() const
        {
                return ptr_;
        }

        operator bool() const
        {
                return ptr_ != nullptr;
        }

        T& get()
        {
                return *ptr_;
        }

        const T& get() const
        {
                return *ptr_;
        }

        ~derived_storage()
        {
                std::destroy_at( ptr_ );
        }

private:
        T*           ptr_;
        storage_type storage_;
};

}  // namespace emlabcpp
