#include "emlabcpp/protocol/decl.h"

#pragma once

namespace emlabcpp
{

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
