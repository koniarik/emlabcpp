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

        constexpr static value_type
        make_val( const typename protocol_item_decl< Args >::value_type&... args )
        {
                return { tag< ID >{}, args... };
        }
};

template < typename... Cmds >
struct protocol_command_group
{
        static_assert(
            are_same_v< typename Cmds::id_type... >,
            "Each command of one group has to use same type of id" );

        using cmds_type  = std::tuple< Cmds... >;
        using def_type   = protocol_group< typename Cmds::def_type... >;
        using pitem_decl = protocol_item_decl< def_type >;
        using value_type = typename pitem_decl::value_type;
        using id_type    = typename std::tuple_element_t< 0, cmds_type >::id_type;

        static constexpr std::size_t max_size = protocol_item_decl< def_type >::max_size;

        using message_type = protocol_message< max_size >;

        template < id_type id, typename... Args >
        requires( ( id == Cmds::id ) || ... ) constexpr static value_type
            make_val( const Args&... args )
        {
                std::optional< value_type > res;

                for_each_index< sizeof...( Cmds ) >( [&]< std::size_t i >() {
                        using cmd = std::tuple_element_t< i, cmds_type >;
                        if constexpr ( cmd::id == id ) {
                                res.emplace( cmd::make_val( args... ) );
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
