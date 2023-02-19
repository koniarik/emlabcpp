#include "emlabcpp/either.h"

#include <cstdint>
#include <functional>

#pragma once

namespace emlabcpp
{

enum class contiguous_tree_type : uint8_t
{
        VALUE  = 1,
        OBJECT = 2,
        ARRAY  = 3
};

enum class contiguous_container_type : uint8_t
{
        ARRAY  = 1,
        OBJECT = 2
};

enum class contiguous_request_adapter_errors : uint8_t
{
        MISSING_NODE  = 1,
        WRONG_TYPE    = 2,
        CHILD_MISSING = 3,
        FULL          = 4
};

}  // namespace emlabcpp
