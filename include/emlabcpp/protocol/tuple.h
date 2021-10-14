#include "emlabcpp/protocol/decl.h"

#pragma once

namespace emlabcpp
{

// protocol_tuple is high levle alternative to use just 'std::tuple' that is more friendly for
// standalone protocols. It is designed for message that are simply a set of serialized items. It
// also provieds more readable syntax. The template arguments at first configurate the protocol and
// later is follow by items present in the tuple, this can also be added with
// protocol_tuple::with_items alias that appends the items. For example:
//
// protocol_tuple< PROTOCOL_BIG_ENDIAN >::with_items< uint32_t, uint32_t >;
//
// serializes/deserializes in same way as 'std::tuple<uint32_t,uint32_t>' configured for big
// endianess.
template < protocol_endianess_enum Endianess, typename... Ds >
struct protocol_tuple : protocol_def_type_base
{
        template < typename... SubDs >
        using with_items = protocol_tuple< Endianess, Ds..., SubDs... >;

        using def_type = protocol_endianess< Endianess, std::tuple< Ds... > >;
        using decl     = protocol_decl< def_type >;

        static constexpr std::size_t max_size = decl::max_size;

        using value_type   = typename decl::value_type;
        using message_type = protocol_message< max_size >;

        constexpr static value_type
        make_val( const typename protocol_decl< Ds >::value_type&... args )
        {
                return value_type{ args... };
        }
};

}  // namespace emlabcpp
