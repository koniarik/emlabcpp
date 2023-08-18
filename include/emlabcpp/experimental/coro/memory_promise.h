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

#pragma once

#include "emlabcpp/experimental/logging.h"
#include "emlabcpp/pmr/memory_resource.h"
#include "emlabcpp/pmr/throw_bad_alloc.h"

namespace emlabcpp::coro
{

template < typename PromiseType >
struct memory_promise
{
        static constexpr std::size_t ptr_size = sizeof( pmr::memory_resource* );

        void*
        operator new( const std::size_t sz, auto&, pmr::memory_resource& pi, auto&&... ) noexcept
        {
                return alloc( sz, pi );
        }

        void* operator new( const std::size_t sz, pmr::memory_resource& pi, auto&&... ) noexcept
        {
                return alloc( sz, pi );
        }

        void* operator new( const std::size_t sz, auto&&... ) noexcept = delete;

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

                const bool succeeded =
                    ( *p )->deallocate( p, size + ptr_size, alignof( PromiseType ) );
                if ( !succeeded ) {
                        pmr::throw_bad_alloc();
                }
        }
};
}  // namespace emlabcpp::coro
