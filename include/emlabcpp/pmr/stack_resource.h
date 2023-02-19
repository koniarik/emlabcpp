#include "emlabcpp/experimental/logging.h"
#include "emlabcpp/pmr/memory_resource.h"
#include "emlabcpp/pmr/util.h"

#include <array>
#include <cstring>

#pragma once

namespace emlabcpp::pmr
{

template < std::size_t Capacity >
class stack_resource final : public pmr::memory_resource
{
        struct node
        {
                std::byte* prev_ptr;
                std::byte* next_ptr;
        };

        static constexpr std::size_t node_size = sizeof( node );
        using buffer                           = std::array< std::byte, Capacity >;

public:
        stack_resource()
        {
                top_ = buff_.data() + node_size;
                set_node( top_, nullptr, nullptr );
        }

        stack_resource( const stack_resource& )            = delete;
        stack_resource( stack_resource&& )                 = delete;
        stack_resource& operator=( const stack_resource& ) = delete;
        stack_resource& operator=( stack_resource&& )      = delete;

        [[nodiscard]] void*
        allocate( const std::size_t bytes, const std::size_t alignment ) override
        {
                EMLABCPP_INFO_LOG( "Top is ", top_, " and start of buffer is ", buff_.data() );

                std::byte* prev_ptr  = top_;
                node       prev_node = get_node( prev_ptr );

                auto* const p = reinterpret_cast< std::byte* >( align( top_, alignment ) );

                top_ = p + bytes + node_size;

                set_node( top_, prev_ptr, nullptr );
                set_node( prev_ptr, prev_node.prev_ptr, top_ );

                EMLABCPP_INFO_LOG( "Allocating ", p, " for ", bytes, " bytes" );

                return p;
        }

        [[nodiscard]] bool
        deallocate( void* const ptr, const std::size_t bytes, const std::size_t ) override
        {
                EMLABCPP_INFO_LOG( "Deallocating ", ptr, " with ", bytes, " bytes" );
                std::byte* node_ptr = reinterpret_cast< std::byte* >( ptr ) + bytes + node_size;
                auto [prev_ptr, next_ptr] = get_node( node_ptr );

                auto [prev_prev_ptr, prev_next_ptr] = get_node( prev_ptr );
                set_node( prev_ptr, prev_prev_ptr, next_ptr );

                if ( next_ptr == nullptr ) {
                        top_ = prev_ptr;
                } else {
                        auto [next_prev_ptr, next_next_ptr] = get_node( next_ptr );
                        set_node( next_ptr, prev_ptr, next_next_ptr );
                }

                return true;
        }

        [[nodiscard]] bool is_equal( const pmr::memory_resource& other ) const noexcept override
        {
                return this == &other;
        }

        [[nodiscard]] bool is_full() const override
        {
                return top_ == buff_.end();
        }

        ~stack_resource() override = default;

private:
        node get_node( std::byte* ptr )
        {
                EMLABCPP_INFO_LOG( "Reading from ", ptr );
                ptr -= sizeof( std::byte* );

                std::byte* prev = nullptr;
                std::memcpy( &prev, ptr, sizeof( std::byte* ) );

                ptr -= sizeof( std::byte* );

                std::byte* next = nullptr;
                std::memcpy( &next, ptr, sizeof( std::byte* ) );
                return { prev, next };
        };

        void set_node( std::byte* ptr, std::byte* const prev, std::byte* const next )
        {
                EMLABCPP_INFO_LOG( "Storing at ", ptr );
                ptr -= sizeof( std::byte* );
                std::memcpy( ptr, &prev, sizeof( std::byte* ) );
                ptr -= sizeof( std::byte* );
                std::memcpy( ptr, &next, sizeof( std::byte* ) );
        }

        std::byte* top_ = nullptr;
        buffer     buff_{};
};

}  // namespace emlabcpp::pmr
