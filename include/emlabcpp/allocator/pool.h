#include "emlabcpp/iterators/numeric.h"
#include "emlabcpp/static_vector.h"
#include "emlabcpp/assert.h"

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
        virtual void* allocate( std::size_t ) = 0;
        virtual void  deallocate( void* )     = 0;

        virtual ~pool_interface() = default;
};

template < std::size_t PoolSize, uint16_t PoolCount >
class pool_resource : public pool_interface
{
public:
        pool_resource()
        {
                for ( std::size_t i : range( PoolCount ) ) {
                        free_.push_back( static_cast< uint16_t >( i ) );
                }
        }

        void* allocate( std::size_t bytes ) final
        {
                if ( free_.empty() || bytes > PoolSize ) {
#ifdef __EXCEPTIONS
                        throw std::bad_alloc{};
#else
                        EMLABCPP_ASSERT( false );
                        return nullptr;
#endif
                }
                std::size_t i = free_.take_back();
                return &pools_[i];
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
        using pool = std::byte[PoolSize];

        static_vector< uint16_t, PoolCount > free_;
        pool                                 pools_[PoolCount];
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

                std::size_t capacity = n + alignof( T );
                void*       res      = resource_->allocate( n * sizeof( T ) + alignof( T ) );
                std::align( alignof( T ), sizeof( T ) * n, res, capacity );
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
