#pragma once

#include <cstdint>

namespace emlabcpp::cfg
{

// chcksum | header | chcksum | payload | [ chkcsum | field ] ...

enum class load_result
{
        SUCCESS,
        DESERIALIZATION_ERROR,
        PAYLOAD_REFUSED
};

using checksum  = uint32_t;
using record_id = uint32_t;

struct header
{
        // number of fields stored after the header
        uint32_t field_count;
};

}  // namespace emlabcpp::cfg
