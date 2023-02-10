
#include "emlabcpp/experimental/logging.h"
#include "emlabcpp/pmr/memory_resource.h"
#include "emlabcpp/pmr/throw_bad_alloc.h"

#pragma once

namespace emlabcpp::coro
{

template < typename PromiseType >
struct memory_promise
{
        static constexpr std::size_t ptr_size = sizeof( pmr::memory_resource* );

        void* operator new( const std::size_t sz, auto&, pmr::memory_resource& pi, auto&&... )
        {
                return alloc( sz, pi );
        }

        void* operator new( const std::size_t sz, pmr::memory_resource& pi, auto&&... )
        {
                return alloc( sz, pi );
        }

        static void* alloc( std::size_t sz, pmr::memory_resource& pi )
        {
                sz += ptr_size;
                void* const vp = pi.allocate( sz, alignof( PromiseType ) );

                auto p = reinterpret_cast< pmr::memory_resource** >( vp );

                *p = &pi;

                p++;

                return p;
        }

        void operator delete( void* const ptr, const std::size_t size )
        {
                auto p = reinterpret_cast< pmr::memory_resource** >( ptr );

                p--;

                const bool succeeded = ( *p )->deallocate( p, size + ptr_size, alignof( PromiseType ) );
                if ( !succeeded ) {
                        pmr::throw_bad_alloc();
                }
        }
};
}  // namespace emlabcpp::coro
