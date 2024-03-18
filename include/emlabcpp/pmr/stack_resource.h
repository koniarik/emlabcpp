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

#include <array>
#include <cstring>

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

        stack_resource( stack_resource const& )            = delete;
        stack_resource( stack_resource&& )                 = delete;
        stack_resource& operator=( stack_resource const& ) = delete;
        stack_resource& operator=( stack_resource&& )      = delete;

        [[nodiscard]] void*
        allocate( std::size_t const bytes, std::size_t const alignment ) override
        {

                std::byte* prev_ptr  = top_;
                node const prev_node = get_node( prev_ptr );

                auto* const p = reinterpret_cast< std::byte* >( align( top_, alignment ) );

                std::byte* new_top = p + bytes + node_size;

                if ( new_top + node_size > buff_.end() )
                        return nullptr;

                top_ = new_top;

                set_node( top_, prev_ptr, nullptr );
                set_node( prev_ptr, prev_node.prev_ptr, top_ );

                EMLABCPP_DEBUG_LOG( "Allocating ", p, " for ", bytes, " bytes" );

                return p;
        }

        [[nodiscard]] result
        deallocate( void* const ptr, std::size_t const bytes, std::size_t const ) override
        {
                EMLABCPP_DEBUG_LOG( "Deallocating ", ptr, " with ", bytes, " bytes" );
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

                return SUCCESS;
        }

        [[nodiscard]] bool is_equal( pmr::memory_resource const& other ) const noexcept override
        {
                return this == &other;
        }

        [[nodiscard]] bool is_full() const noexcept override
        {
                return top_ == buff_.end();
        }

        ~stack_resource() override = default;

private:
        node get_node( std::byte* ptr )
        {
                ptr -= sizeof( std::byte* );

                std::byte* prev = nullptr;
                std::memcpy( &prev, ptr, sizeof( std::byte* ) );

                ptr -= sizeof( std::byte* );

                std::byte* next = nullptr;
                std::memcpy( &next, ptr, sizeof( std::byte* ) );
                return { prev, next };
        };

        void set_node( std::byte* ptr, std::byte* const prev, std::byte* const next ) const
        {
                ptr -= sizeof( std::byte* );
                std::memcpy( ptr, &prev, sizeof( std::byte* ) );
                ptr -= sizeof( std::byte* );
                std::memcpy( ptr, &next, sizeof( std::byte* ) );
        }

        std::byte* top_ = nullptr;
        buffer     buff_{};
};

}  // namespace emlabcpp::pmr
