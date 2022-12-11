#include "emlabcpp/pmr/memory_resource.h"
#include "emlabcpp/pmr/throw_bad_alloc.h"

#pragma once

namespace emlabcpp::pmr
{

template < typename T >
class allocator
{
public:
        using value_type = T;

        template < typename U >
        allocator( const allocator< U >& other )
          : resource_( other.resource_ )
        {
        }

        allocator( pmr::memory_resource& src )
          : resource_( src )
        {
        }

        T* allocate( const std::size_t n )
        {

                void* const res = resource_.get().allocate( n * sizeof( T ), alignof( T ) );
                if ( res == nullptr ) {
                        throw_bad_alloc();
                }
                return reinterpret_cast< T* >( res );
        }

        void deallocate( T* const p, const std::size_t size ) noexcept
        {
                const bool succeeded = resource_.get().deallocate(
                    reinterpret_cast< void* >( p ), size, alignof( T ) );
                if ( !succeeded ) {
                        throw_bad_alloc();
                }
        }

        friend constexpr bool operator==( const allocator& lh, const allocator& rh )
        {
                return lh.resource_.get().is_equal( rh.resource_.get() );
        }
        friend constexpr bool operator!=( const allocator& lh, const allocator& rh )
        {
                return !( lh == rh );
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
