#include "emlabcpp/pmr/memory_resource.h"
#include "emlabcpp/pmr/util.h"
#include "emlabcpp/range.h"
#include "emlabcpp/static_vector.h"

#pragma once

namespace emlabcpp::pmr
{

template < std::size_t PoolSize, uint16_t PoolCount >
class pool_resource final : public pmr::memory_resource
{
public:
        pool_resource()
        {
                for ( const std::size_t i : range( PoolCount ) ) {
                        free_.push_back( static_cast< uint16_t >( i ) );
                }
        }

        pool_resource( const pool_resource& )            = delete;
        pool_resource( pool_resource&& )                 = delete;
        pool_resource& operator=( const pool_resource& ) = delete;
        pool_resource& operator=( pool_resource&& )      = delete;

        [[nodiscard]] void*
        allocate( const std::size_t bytes, const std::size_t alignment ) override
        {
                void*       p    = nullptr;
                std::size_t used = bytes;
                if ( !free_.empty() ) {
                        const std::size_t i      = free_.back();
                        void* const       pool_p = pools_[i].data();
                        p                        = align( pool_p, alignment );
                        used += static_cast< std::size_t >(
                            reinterpret_cast< std::byte* >( p ) -
                            reinterpret_cast< std::byte* >( pool_p ) );
                }

                if ( p == nullptr || used > PoolSize ) {
                        EMLABCPP_ERROR_LOG( "Failed to allocate ", bytes, " bytes" );
                        return nullptr;
                }
                free_.pop_back();
                return p;
        }

        [[nodiscard]] bool
        deallocate( void* const ptr, const std::size_t, const std::size_t ) override
        {

                const auto pval = reinterpret_cast< std::size_t >( ptr );
                const auto bval = reinterpret_cast< std::size_t >( &pools_ );

                const std::size_t spot_i = ( pval - bval ) / PoolSize;

                if ( spot_i >= PoolCount ) {
                        EMLABCPP_ERROR_LOG( "Failed to deallocate" );
                        return false;
                }
                free_.push_back( static_cast< uint16_t >( spot_i ) );
                return true;
        }

        [[nodiscard]] bool is_equal( const pmr::memory_resource& other ) const noexcept override
        {
                return this == &other;
        }

        [[nodiscard]] bool is_full() const override
        {
                return free_.empty();
        }

        ~pool_resource() override = default;

private:
        using pool = std::array< std::byte, PoolSize >;

        static_vector< uint16_t, PoolCount > free_;
        std::array< pool, PoolCount >        pools_;
};

}  // namespace emlabcpp::pmr
