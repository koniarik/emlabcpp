// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//
//  Copyright © 2022 Jan Veverak Koniarik
//  This file is part of project: emlabcpp
//
#include "emlabcpp/allocator/util.h"
#include "emlabcpp/assert.h"
#include "emlabcpp/iterators/numeric.h"
#include "emlabcpp/static_vector.h"

#include <deque>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <vector>

#pragma once

namespace emlabcpp
{

struct pool_interface
{
        virtual void* allocate( std::size_t bytes, std::size_t alignment ) = 0;
        virtual void  deallocate( void* )                                  = 0;

        virtual ~pool_interface() = default;
};

template < std::size_t PoolSize, uint16_t PoolCount >
class pool_resource final : public pool_interface
{
public:
        pool_resource()
        {
                for ( std::size_t i : range( PoolCount ) ) {
                        free_.push_back( static_cast< uint16_t >( i ) );
                }
        }

        pool_resource( const pool_resource& )            = delete;
        pool_resource( pool_resource&& )                 = delete;
        pool_resource& operator=( const pool_resource& ) = delete;
        pool_resource& operator=( pool_resource&& )      = delete;

        void* allocate( std::size_t bytes, std::size_t alignment ) final
        {
                void*       p    = nullptr;
                std::size_t used = bytes;
                if ( !free_.empty() ) {
                        std::size_t i      = free_.back();
                        void*       pool_p = &pools_[i];
                        p                  = align( pool_p, alignment );
                        used += static_cast< std::size_t >(
                            reinterpret_cast< std::byte* >( p ) -
                            reinterpret_cast< std::byte* >( pool_p ) );
                }

                if ( !p || used > PoolSize ) {
#ifdef __EXCEPTIONS
                        throw std::bad_alloc{};
#else
                        EMLABCPP_ASSERT( false );
                        return nullptr;
#endif
                }
                free_.pop_back();
                return p;
        }

        void deallocate( void* ptr ) final
        {

                auto pval = reinterpret_cast< std::size_t >( ptr );
                auto bval = reinterpret_cast< std::size_t >( &pools_ );

                std::size_t spot_i = ( pval - bval ) / PoolSize;

                if ( spot_i >= PoolCount ) {
#ifdef __EXCEPTIONS
                        throw std::bad_alloc{};
#else
                        EMLABCPP_ASSERT( false );
                        return;
#endif
                }
                free_.push_back( static_cast< uint16_t >( spot_i ) );
        }

        ~pool_resource() final = default;

private:
        using pool = std::array< std::byte, PoolSize >;

        static_vector< uint16_t, PoolCount > free_;
        std::array< pool, PoolCount >        pools_;
};

template < typename T >
class pool_allocator
{
public:
        using value_type = T;

        template < typename U >
        pool_allocator( const pool_allocator< U >& other )
          : resource_( other.resource_ )
        {
        }

        pool_allocator( pool_interface* src )
          : resource_( src )
        {
        }

        T* allocate( std::size_t n )
        {

                void* res = resource_->allocate( n * sizeof( T ), alignof( T ) );
                return reinterpret_cast< T* >( res );
        }

        void deallocate( T* p, std::size_t ) noexcept
        {
                resource_->deallocate( p );
        }

        friend constexpr bool operator==( const pool_allocator& lh, const pool_allocator& rh )
        {
                return lh.resource_ == rh.resource_;
        }
        friend constexpr bool operator!=( const pool_allocator& lh, const pool_allocator& rh )
        {
                return !( lh == rh );
        }

        pool_interface* get_resource()
        {
                return resource_;
        }

        template < typename U >
        friend class pool_allocator;

private:
        pool_interface* resource_;
};

template < std::size_t PoolSize, uint16_t PoolCount >
struct pool_base
{
protected:
        pool_resource< PoolSize, PoolCount > pool_memory;
};

struct pool_deleter
{
        pool_interface* res;

        template < typename T >
        void operator()( T* item )
        {
                std::destroy_at( item );
                res->deallocate( item );
        }
};

template < typename T >
using pool_unique_ptr = std::unique_ptr< T, pool_deleter >;

template < typename T >
using pool_vector = std::vector< T, pool_allocator< T > >;
template < typename T >
using pool_list = std::list< T, pool_allocator< T > >;
template < typename T >
using pool_set = std::set< T, pool_allocator< T > >;
template < typename T >
using pool_deque = std::deque< T, pool_allocator< T > >;
template < typename Key, typename T >
using pool_map = std::map< Key, T, std::less< Key >, pool_allocator< std::pair< const Key, T > > >;

}  // namespace emlabcpp
