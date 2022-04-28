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

        derived_storage() = delete;

        template < std::derived_from< T > U >
        derived_storage( U&& item )
          : ptr_( &storage_ )
        {
                ptr_ = align( ptr_, alignof( U ) );
                ::new ( ptr_ ) U( std::forward< U >( item ) );
        }

        derived_storage( const derived_storage& ) = delete;
        derived_storage( derived_storage&& )      = delete;

        derived_storage& operator=( const derived_storage& ) = delete;
        derived_storage& operator=( derived_storage&& )      = delete;

        T& operator*()
        {
                return get_ref();
        }

        const T& operator*() const
        {
                return get_ref();
        }

        T* operator->()
        {
                return &get_ref();
        }

        const T* operator->() const
        {
                return &get_ref();
        }
        T& get()
        {
                return get_ref();
        }

        const T& get() const
        {
                return get_ref();
        }

        ~derived_storage()
        {
                ::delete ( &get_ref() );
        }

private:
        void*        ptr_;
        storage_type storage_;

        T& get_ref()
        {
                return *reinterpret_cast< T* >( ptr_ );
        }

        const T& get_ref() const
        {
                return *reinterpret_cast< const T* >( ptr_ );
        }
};

}  // namespace emlabcpp
