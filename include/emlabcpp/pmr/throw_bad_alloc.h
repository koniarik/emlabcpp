#include <cstdlib>
#include <new>

#pragma once

namespace emlabcpp::pmr
{

inline void throw_bad_alloc()
{
        // TODO: this needs customization point /o\...
#ifdef __EXCEPTIONS
        throw std::bad_alloc{};
#else
        std::abort();
#endif
}

}  // namespace emlabcpp::pmr
