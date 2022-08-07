
#pragma once

namespace emlabcpp
{

enum contiguous_tree_type_enum
{
        CONTIGUOUS_TREE_VALUE  = 0,
        CONTIGUOUS_TREE_OBJECT = 1,
        CONTIGUOUS_TREE_ARRAY  = 2
};

enum contiguous_container_type
{
        CONTIGUOUS_CONT_ARRAY,
        CONTIGUOUS_CONT_OBJECT
};

enum contiguous_request_adapter_errors_enum
{
        CONTIGUOUS_MISSING_NODE,
        CONTIGUOUS_WRONG_TYPE,
        CONTIGUOUS_CHILD_MISSING,
        CONTIGUOUS_FULL
};

}  // namespace emlabcpp
