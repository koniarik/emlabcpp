#include <cstddef>

#pragma once

namespace emlabcpp::pmr
{
class memory_resource
{
public:
        virtual void* allocate( std::size_t bytes, std::size_t alignment )              = 0;
        virtual bool  deallocate( void* ptr, std::size_t bytes, std::size_t alignment ) = 0;
        virtual bool  is_equal( const memory_resource& other ) const noexcept           = 0;
        virtual bool  is_full() const                                                   = 0;
        virtual ~memory_resource()                                                      = default;
};
}  // namespace emlabcpp::pmr
