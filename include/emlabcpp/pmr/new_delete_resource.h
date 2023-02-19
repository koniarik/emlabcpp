#include "emlabcpp/pmr/memory_resource.h"

#pragma once

namespace emlabcpp::pmr
{
struct new_delete_resource_impl : memory_resource
{
        void* allocate( std::size_t bytes, std::size_t alignment ) override
        {
                return ::operator new ( bytes, std::align_val_t{ alignment } );
        };

        bool deallocate( void* p, std::size_t, std::size_t alignment ) override
        {
                ::operator delete ( p, std::align_val_t{ alignment } );
                return true;
        };

        [[nodiscard]] bool is_equal( const memory_resource& ) const noexcept override
        {
                return true;
        };

        [[nodiscard]] bool is_full() const override
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
