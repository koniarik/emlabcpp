#include "emlabcpp/algorithm.h"
#include "emlabcpp/iterators/numeric.h"

#include <bitset>
#include <memory_resource>

#pragma once

namespace emlabcpp
{
template < std::size_t BucketCount, std::size_t BucketSize >
class bucket_memory_resource : public std::pmr::memory_resource
{
public:
        static constexpr std::size_t bucket_count = BucketCount;
        static constexpr std::size_t bucket_size  = BucketSize;

        std::size_t used() const
        {
                return taken_.count();
        }

protected:
        void* do_allocate( std::size_t bytes, std::size_t alignment ) final
        {
                if ( taken_.all() || bytes + alignment > BucketSize ) {
#ifdef __EXCEPTIONS
                        throw std::bad_alloc{};
#else
                        EMLABCPP_ASSERT( false );
                        return nullptr;
#endif
                }

                std::size_t spot_i = *find_if( range( taken_.size() ), [&]( std::size_t i ) {
                        return !taken_[i];
                } );

                taken_[spot_i]    = true;
                void*       mem   = buckets_[spot_i];
                std::size_t space = bucket_size;
                return std::align( alignment, bytes, mem, space );
        }

        void do_deallocate( void* p, std::size_t, std::size_t ) final
        {
                auto pval = reinterpret_cast< std::size_t >( p );
                auto bval = reinterpret_cast< std::size_t >( &buckets_ );

                std::size_t spot_i = ( pval - bval ) / bucket_size;

                if ( spot_i >= bucket_count ) {
#ifdef __EXCEPTIONS
                        throw std::bad_alloc{};
#else
                        EMLABCPP_ASSERT( false );
                        return;
#endif
                }
                taken_[spot_i] = 0;
        }

        bool do_is_equal( const std::pmr::memory_resource& other ) const noexcept final
        {
                return this == &other;
        }

private:
        using bucket = std::byte[bucket_size];
        std::bitset< bucket_count > taken_;
        bucket                      buckets_[bucket_count];
};
}  // namespace emlabcpp
