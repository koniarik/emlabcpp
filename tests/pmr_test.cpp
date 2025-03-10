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

#include "emlabcpp/pmr/aliases.h"
#include "emlabcpp/pmr/stack_resource.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

TEST( PMR, stack_resource_list )
{
        pmr::stack_resource< 1024 > stack;

        pmr::list< std::string > l{ stack };

        l.emplace_back( "wololo" );
}

TEST( PMR, stack_resource_list_complex )
{
        pmr::stack_resource< 1024 > stack;

        pmr::list< std::string > l{ stack };

        l.emplace_back( "wololo" );
        l.emplace_back( "tololo" );

        EXPECT_EQ( l.front(), "wololo" );
        EXPECT_EQ( l.back(), "tololo" );

        l.pop_back();

        EXPECT_EQ( l.front(), l.back() );

        l.emplace_back( "kololo" );

        EXPECT_EQ( l.front(), "wololo" );
        EXPECT_EQ( l.back(), "kololo" );
}

struct : pmr::memory_resource
{
        void* allocate( std::size_t, std::size_t )
        {
                return nullptr;
        }

        result deallocate( void*, std::size_t, std::size_t )
        {
                return ERROR;
        }

        bool is_equal( const memory_resource& ) const noexcept
        {
                return true;
        };

        bool is_full() const noexcept
        {
                return true;
        }
} FAILING_RESOURCE;

TEST( PMR, allocator_throws )
{
        pmr::allocator< float > alloc{ FAILING_RESOURCE };

        EXPECT_THROW( { alloc.allocate( 42 ); }, std::bad_alloc );
        EXPECT_THROW( { alloc.deallocate( nullptr, 42 ); }, std::bad_alloc );
}

}  // namespace emlabcpp
