#include "emlabcpp/assert.h"
#include "emlabcpp/protocol/base.h"
#include "emlabcpp/types.h"

#pragma once

namespace emlabcpp
{

template < auto ID, typename... Args >
struct protocol_command
{
        using id_type = decltype( ID );
        using value_type =
            std::tuple< tag< ID >, typename protocol_item_decl< Args >::value_type... >;
        using def_type = std::tuple< tag< ID >, Args... >;

        static constexpr id_type id = ID;

        template < typename... NewArgs >
        using with_args = protocol_command< ID, Args..., NewArgs... >;
};

template < typename... Cmds >
struct protocol_command_group
{
        static_assert( are_same_v< typename Cmds::id_type... > );

        using def_type   = protocol_group< typename Cmds::def_type... >;
        using pitem_decl = protocol_item_decl< def_type >;
        using value_type = typename pitem_decl::value_type;

        static constexpr std::size_t max_size = protocol_item_decl< def_type >::max_size;

        using message_type = protocol_message< max_size >;

        // TODO: that ID type should be known
        // TODO: make proper static check that at least one cmd matches the ID

        template < auto id, typename... Args >
        static value_type make_val( Args... args )
        {
                std::optional< value_type > res;

                for_each_index< sizeof...( Cmds ) >( [&]< std::size_t i >() {
                        using cmd = std::tuple_element_t< i, std::tuple< Cmds... > >;
                        using T   = typename cmd::value_type;
                        if constexpr ( cmd::id == id ) {
                                res.emplace( T{ tag< id >{}, args... } );
                        }
                } );

                EMLABCPP_ASSERT( res );
                return *res;
        }
};

template <>
struct protocol_command_group<>
{
        template < typename... NewCommands >
        using with_commands = protocol_command_group< NewCommands... >;
};

}  // namespace emlabcpp
