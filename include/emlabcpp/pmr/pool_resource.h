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
#include "emlabcpp/pmr/util.h"
#include "emlabcpp/range.h"
#include "emlabcpp/static_vector.h"

namespace emlabcpp::pmr
{

template < std::size_t PoolSize, uint16_t PoolCount >
class pool_resource final : public pmr::memory_resource
{
public:
        pool_resource()
        {
                for ( const std::size_t i : range( PoolCount ) )
                        free_.push_back( static_cast< uint16_t >( i ) );
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

        [[nodiscard]] result
        deallocate( void* const ptr, const std::size_t, const std::size_t ) override
        {

                const auto pval = std::bit_cast< std::size_t >( ptr );
                const auto bval = std::bit_cast< std::size_t >( &pools_ );

                const std::size_t spot_i = ( pval - bval ) / PoolSize;

                if ( spot_i >= PoolCount ) {
                        EMLABCPP_ERROR_LOG( "Failed to deallocate" );
                        return ERROR;
                }
                free_.push_back( static_cast< uint16_t >( spot_i ) );
                return SUCCESS;
        }

        [[nodiscard]] bool is_equal( const pmr::memory_resource& other ) const noexcept override
        {
                return this == &other;
        }

        [[nodiscard]] bool is_full() const noexcept override
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
