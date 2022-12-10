
#include "emlabcpp/allocator/pool.h"

#pragma once

namespace emlabcpp::coro
{

template < typename PromiseType >
struct pool_promise
{
        static constexpr std::size_t ptr_size = sizeof( pool_interface* );

        void* operator new( std::size_t sz, auto&, pool_interface* pi, auto&&... )
        {
                return alloc( sz, pi );
        }

        void* operator new( std::size_t sz, pool_interface* pi, auto&&... )
        {
                return alloc( sz, pi );
        }

        static void* alloc( std::size_t sz, pool_interface* pi )
        {
                sz += ptr_size;
                void* vp = pi->allocate( sz, alignof( PromiseType ) );

                auto p = reinterpret_cast< pool_interface** >( vp );

                *p = pi;

                p++;

                return p;
        }

        void operator delete( void* ptr, std::size_t )
        {
                auto p = reinterpret_cast< pool_interface** >( ptr );

                p--;

                ( *p )->deallocate( ptr, alignof( PromiseType ) );
        }
};
}  // namespace emlabcpp::coro
