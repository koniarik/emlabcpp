#include "emlabcpp/bounded.h"
#include "emlabcpp/protocol/error.h"

#include <bitset>
#include <cstring>
#include <type_traits>
#include <variant>

#pragma once

namespace emlabcpp
{

// Strucutre used as result of deserialization in the internal mechanisms of protocol handling.
// Contains parsed value and bounded value of how much bytes were used.
template < bounded_derived size_type, typename T >
struct protocol_result
{
        size_type used;
        T         val;
};

template < bounded_derived size_type, typename T >
protocol_result( size_type, T ) -> protocol_result< size_type, T >;

// Concept that matches types considered base - serialized directly by using byte shifting.
template < typename T >
concept protocol_base_type = std::is_integral_v< T > || std::is_enum_v< T >;

// Enum specifies what endianess should be used.
enum protocol_endianess_enum
{
        PROTOCOL_BIG_ENDIAN,
        PROTOCOL_LITTLE_ENDIAN
};
// TODO: when able to move to GCC11: using enum protocol_endianess_enum; and make it enum class

// Follows a set of special data types used for definition of protocol. These either represent
// special types or affect the serialization/deserialization process of normal types.
// -----------------------------------------------------------------------------------------------

// Changes the endianess of definition D.
template < protocol_endianess_enum Endianess, typename D >
struct protocol_endianess
{
        static constexpr protocol_endianess_enum value = Endianess;
        using value_type                               = D;
};

// Serializes values from definitions Ds to std::variant. The byte message does not contain
// identificator of variant used, rather the first definition that manages to deserialize the
// message is used.
template < typename... Ds >
struct protocol_group
{
        using options_type = std::variant< Ds... >;
};

// Creates a segment starting with counter defined by CounterDef, this counter limits how many bytes
// are passed to deserialization process, bytes after the limit ale not considered by this segment.
template < typename CounterDef, typename D >
struct protocol_sized_buffer
{
        using counter_type = CounterDef;
        using value_type   = D;
};

// The value defined by `D` present in the message is offseted by `Offset`. If the offset for
// example `2`, value `4` in the message is parsed as `2` and value `1` is serialized as `3`.
template < typename D, auto Offset >
struct protocol_offset
{
        static constexpr auto offset = Offset;
        using def_type               = D;
};

// More complex constructs have custom mechanics that internally produces `def_type` alias used by
// the library to serialize/deserialize it. Type inheriting htis class are handled as their
// `def_type`.
struct protocol_def_type_base
{
};

}  // namespace emlabcpp
