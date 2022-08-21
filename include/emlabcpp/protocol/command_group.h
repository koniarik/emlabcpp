// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//
//  Copyright © 2022 Jan Veverak Koniarik
//  This file is part of project: emlabcpp
//
#include "emlabcpp/assert.h"
#include "emlabcpp/protocol/decl.h"
#include "emlabcpp/types.h"

#pragma once

namespace emlabcpp::protocol
{

/// Command group represents a segment in the message, that may contain multiple different variants
/// of value, that are identified by ID at the beginning of the segment. The ID is defined in the
/// definition.
//
/// This simulates abstraction of `set of commands that can be sent` that is frequently used by
/// embedded devices.

/// One command in a group defined by ID and definitions for items contained in this command. It is
/// preferable to use the `protocol_command::with_args<...>` alias that just extens the list of
/// defined items in the command. For example: protocol_command<32>::with_args<uint32_t, uint32_t>
//
/// Internally, the example leads to definition `std::tuple< tag<32>, uint32_t, uint32_t >`
template < auto ID, protocol_declarable... Defs >
struct protocol_command
{
        using id_type    = decltype( ID );
        using value_type = std::tuple< tag< ID >, typename protocol_decl< Defs >::value_type... >;
        using def_type   = std::tuple< tag< ID >, Defs... >;

        static constexpr id_type id = ID;

        template < typename... NewDefs >
        using with_args = protocol_command< ID, Defs..., NewDefs... >;

        /// Creates value of the command based on the args.
        constexpr static value_type
        make_val( const typename protocol_decl< Defs >::value_type&... args )
        {
                return { tag< ID >{}, args... };
        }
};

/// Command group should be used as a collection of commands that are selected based on the ID,
/// during the deserialization first command that matches is selected. The command_group acts as
/// def_type that is handled by the protocol library. The internal definition produced is just
/// std::variant of tuples that form the commands itself.
//
/// Note that group without commands is not usable by the protocol library as that case is
/// specialized and doesn ot contain any protocol definitions. This is required implementation
/// detail.
template < protocol_endianess_enum Endianess, typename... Cmds >
struct protocol_command_group : protocol_def_type_base
{
        using cmds_type = std::tuple< Cmds... >;

private:
        // TODO: duplication from register_map
        static constexpr std::size_t get_id_index( auto id )
        {
                std::size_t res = sizeof...( Cmds );
                until_index< sizeof...( Cmds ) >( [&]< std::size_t i >() {
                        if ( std::tuple_element_t< i, cmds_type >::id == id ) {
                                res = i;
                                return true;
                        }
                        return false;
                } );
                return res;
        }

public:
        template < auto ID >
        static constexpr std::size_t id_index = get_id_index( ID );

        template < auto ID >
        using cmd_type = std::tuple_element_t< id_index< ID >, cmds_type >;

        template < auto ID >
        using cmd_value_type = typename cmd_type< ID >::value_type;

        template < typename... SubCmds >
        using with_commands = protocol_command_group< Endianess, Cmds..., SubCmds... >;

        static_assert(
            are_same_v< typename Cmds::id_type... >,
            "Each command of one group has to use same type of id" );

        using def_type =
            protocol_endianess< Endianess, protocol_group< typename Cmds::def_type... > >;
        using decl       = protocol_decl< def_type >;
        using value_type = typename decl::value_type;

        static constexpr std::size_t max_size = protocol_decl< def_type >::max_size;

        using message_type = protocol_message< max_size >;

        /// Creates value of the command group, that is variant with value of the command 'id' that
        /// will receive the appropiate 'args'.
        template < auto id, typename... Args >
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

template < protocol_endianess_enum Endianess >
struct protocol_command_group< Endianess >
{
        template < typename... Cmds >
        using with_commands = protocol_command_group< Endianess, Cmds... >;
};

}  // namespace emlabcpp::protocol
