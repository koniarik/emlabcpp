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

#include "emlabcpp/pmr/memory_resource.h"

#include <cstddef>
#include <new>

namespace emlabcpp::pmr
{
struct new_delete_resource_impl : memory_resource
{
        void* allocate( std::size_t bytes, std::size_t alignment ) override
        {
                return ::operator new( bytes, std::align_val_t{ alignment } );
        };

        bool deallocate( void* p, std::size_t, std::size_t alignment ) override
        {
                ::operator delete( p, std::align_val_t{ alignment } );
                return true;
        };

        [[nodiscard]] bool is_equal( const memory_resource& ) const noexcept override
        {
                return true;
        };

        [[nodiscard]] bool is_full() const noexcept override
        {
                return false;
        }
};

inline memory_resource& new_delete_resource()
{
        static new_delete_resource_impl impl;
        return impl;
}
}  // namespace emlabcpp::pmr
