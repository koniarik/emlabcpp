#pragma once

namespace emlabcpp
{

template < protocol_endianess_enum Endianess, typename... Ts >
struct protocol_tuple : protocol_def_type_base
{

        using def_type  = protocol_endianess< Endianess, std::tuple< Ts... > >;
        using item_decl = protocol_item_decl< def_type >;

        static constexpr std::size_t max_size = item_decl::max_size;

        using value_type   = typename item_decl::value_type;
        using message_type = protocol_message< max_size >;

        constexpr static value_type
        make_val( const typename protocol_item_decl< Ts >::value_type&... args )
        {
                return value_type{ args... };
        }
};

}  // namespace emlabcpp
