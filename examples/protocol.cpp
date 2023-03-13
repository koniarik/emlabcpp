///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
/// and associated documentation files (the "Software"), to deal in the Software without
/// restriction, including without limitation the rights to use, copy, modify, merge, publish,
/// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or
/// substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
/// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
/// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#include "emlabcpp/protocol.h"

#include "emlabcpp/protocol/handler.h"
#include "emlabcpp/protocol/streams.h"

#include <iostream>

namespace em = emlabcpp;

int main( int, char*[] )
{

        // ---------------------------------------------------------------------------------------
        // protocol library allows serialization and deserialization of C++ native types into binary
        // messages. The protocol is defined with types from the library and the library provides
        // handler that does the conversion based on that definition. The definition is done using
        // types and templates and is separated from the actuall serialization and deserialization.
        //
        // There are two top level types that shall be used: protocol::tuple and
        // protocol::command_group (variant-like). The user should use these as top level
        // type and define the protocol in these. They can be nested.
        //
        // For the examples we define the protocol by definining structures that inherit from the
        // protocol definition, this is not necessary and simple aliases to types can be used, these
        // however produce lengthy error messages.
        //
        // For both types we also use syntax sugar in form of type alias members such as
        // 'with_items' that make the definition more readable.

        // ---------------------------------------------------------------------------------------
        // The protocol tuple is defined by endianess and items that are stored in the tuple.
        // protocol::tuple uses std::tuple as the type that actually holds the value. The
        // example definition below defines protocol for converting `std::tuple<uint32_t, in16_t,
        // int16_t>` into binary message `protocol::message<8>` of size 8.
        //
        // The structure contains ::value_type and ::message_type aliases.
        struct example_tuple
          : em::protocol::tuple< std::endian::big >::with_items< uint32_t, int16_t, int16_t >
        {
        };

        // Once protocol is defined, it can be used with protocol::handler to do the
        // serialization and deserialization. The materialization of code for handling the protocol
        // happens in the handler itself. The compile time is costly for the handler, it is
        // preferable to use it in standalone compilation unit.
        using example_tuple_handler = em::protocol::handler< example_tuple >;

        std::tuple< uint32_t, int16_t, int16_t > tuple_val = { 666, -2, 2 };
        em::protocol::message< 8 > tuple_msg = example_tuple_handler::serialize( tuple_val );

        // The library has support for streams, these however are stored in separate included file
        // and has to be enable by defining EMLABCPP_USE_STREAMS
        std::cout << "Message from example_tuple looks like: " << tuple_msg << "\n";
        // The message output is: |00:00:02:9a|ff:fe:00:02

        // Deserialization produces either the value on successfull serialization or
        // protocol::error_record with information about what failed and on which byte.
        // Note: the protocol::error_record is serializable by the library, so you can
        // simply send it in report.

        em::either< std::tuple< uint32_t, int16_t, int16_t >, em::protocol::error_record >
            tuple_either = example_tuple_handler::extract( tuple_msg );

        tuple_either.match(
            []( std::tuple< uint32_t, int16_t, int16_t > ) {
                    std::cout << "Yaaay, protocol deserialized what it serialized \\o/\n";
            },
            []( em::protocol::error_record rec ) {
                    std::cout << "Hupsie, error happend in deserialization: " << rec << "\n";
            } );

        // ---------------------------------------------------------------------------------------
        // command_group models situation of "message contains one out of many commands", where
        // command is just specific tuple of types. This produces std::variant of possible commands
        // on deserialization and convert said variant to byte message. The first item of each
        // command is constant (em::tag) with id of the command. The library decides which command
        // is in the message solely based on this id. These however does not have to be ordered as
        // seen in the example.

        enum protocol_id : uint16_t
        {
                EXAMPLE_CMD_A = 1,
                EXAMPLE_CMD_B = 42,
                EXAMPLE_CMD_C = 66,
        };

        struct example_group
          : em::protocol::command_group< std::endian::big >::with_commands<
                em::protocol::command< EXAMPLE_CMD_A >::with_args< uint32_t >,
                em::protocol::command< EXAMPLE_CMD_B >,
                em::protocol::command< EXAMPLE_CMD_C >::with_args<
                    std::array< uint32_t, 4 >,
                    std::tuple< uint8_t, uint8_t, em::bounded< int16_t, -666, 666 > > > >
        {
        };

        // This command group produces value defined below:

        using example_group_value = std::variant<
            std::tuple< em::tag< EXAMPLE_CMD_A >, uint32_t >,
            std::tuple< em::tag< EXAMPLE_CMD_B > >,
            std::tuple<
                em::tag< EXAMPLE_CMD_C >,
                std::array< uint32_t, 4 >,
                std::tuple< uint8_t, uint8_t, em::bounded< int16_t, -666, 666 > > > >;

        // To simplify the process of handling the value, the command_group provides make_val static
        // method for creating a value of said group, that can be processed.

        example_group_value group_val = example_group::make_val< EXAMPLE_CMD_A >( 42u );

        // serialization and deserialization works same way as in case of tuple

        using example_group_handler = em::protocol::handler< example_group >;

        em::protocol::message< 22 > group_msg = example_group_handler::serialize( group_val );

        std::cout << "Message from example group looks like: " << group_msg << "\n";
        // this produces: |00:01:00:00|00:2a

        example_group_handler::extract( group_msg )
            .match(
                [&]( example_group_value ) {
                        std::cout << "yayyy, group deserialize :) \n";
                },
                [&]( em::protocol::error_record rec ) {
                        std::cout << "something went wrong with the group: " << rec << "\n";
                } );
}
