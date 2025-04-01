/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
///

#pragma once

#include <cstddef>
#include <memory>
#include <utility>

namespace emlabcpp
{

/// Continuous data container that can contain N of uninitialized elements. Bookkeeping (what item
/// is initialized and what is not) is up to owner of this structure.
template < typename T, std::size_t N >
class static_storage
{
public:
        static constexpr std::size_t capacity = N;

        using value_type      = T;
        using reference       = T&;
        using const_reference = T const&;
        using pointer         = T*;
        using const_pointer   = T const*;
        using size_type       = std::size_t;

        /// Returns pointer to first item of the storage
        [[nodiscard]] constexpr pointer data() noexcept
        {
                return reinterpret_cast< pointer >( data_ );
        }

        /// Returns pointer to first item of the storage
        [[nodiscard]] constexpr const_pointer data() const noexcept
        {
                return reinterpret_cast< const_pointer >( data_ );
        }

        /// Constructs an item at position i with arguments args...
        template < typename... Args >
        constexpr T& emplace_item( size_type const i, Args&&... args ) noexcept(
            std::is_nothrow_constructible_v< T, Args... > )
        {
                return *std::construct_at( data() + i, std::forward< Args >( args )... );
        }

        /// Deconstructs an item at position i
        constexpr void
        delete_item( size_type const i ) noexcept( std::is_nothrow_destructible_v< T > )
        {
                std::destroy_at( data() + i );
        }

        /// Provides a reference to item at position i
        [[nodiscard]] constexpr reference operator[]( size_type const i ) noexcept
        {
                return *( data() + i );
        }

        /// Provides a reference to item at position i
        [[nodiscard]] constexpr const_reference operator[]( size_type const i ) const noexcept
        {
                return *( data() + i );
        }

private:
        alignas( T ) std::byte data_[N * sizeof( T )];
};

}  // namespace emlabcpp
