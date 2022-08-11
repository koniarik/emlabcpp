
#pragma once

namespace emlabcpp
{

enum contiguous_tree_type_enum : uint8_t
{
        CONTIGUOUS_TREE_VALUE  = 1,
        CONTIGUOUS_TREE_OBJECT = 2,
        CONTIGUOUS_TREE_ARRAY  = 3
};

enum contiguous_container_type : uint8_t
{
        CONTIGUOUS_CONT_ARRAY  = 1,
        CONTIGUOUS_CONT_OBJECT = 2
};

enum contiguous_request_adapter_errors_enum : uint8_t
{
        CONTIGUOUS_MISSING_NODE  = 1,
        CONTIGUOUS_WRONG_TYPE    = 2,
        CONTIGUOUS_CHILD_MISSING = 3,
        CONTIGUOUS_FULL          = 4
};

}  // namespace emlabcpp
