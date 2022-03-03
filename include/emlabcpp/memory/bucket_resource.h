
#pragma once

namespace emlabcpp
{
template < std::size_t BucketCount, std::size_t BUcketSize >
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
#ifdef __EXCEPTIONS
                if ( taken_.all() ) {
                        throw std::bad_alloc{};
                }
#else
                EMLABCPP_ASSERT( !taken_.all() );
#endif

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
                std::size_t spot_i = buffer_count;
                for ( std::size_t i : em::range( bucket_count ) ) {
                        std::size_t reverse_i = bucket_count - 1 - i;
                        if ( buckets_[reverse_i] <= p ) {
                                spot_i = reverse_i;
                                break;
                        }
                }

#ifdef __EXCEPTIONS
                if ( spot_i == bucket_count ) {
                        throw std::bad_alloc{};
                }
#else
                EMLABCPP_ASSERT( spot_i != bucket_count );
#endif
                taken_[spot_i] = 0;
        }

        bool do_is_equal( const std::pmr::memory_resource& other ) const noexcept final
        {
                return this == &other;
        }

private:
        using bucket = std::byte[bucket_size];
        bucket       = buckets_[bucket_count];
        std::bitset< bucket_count > taken _;
};
}  // namespace emlabcpp
