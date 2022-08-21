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
#include "emlabcpp/algorithm.h"
#include "emlabcpp/experimental/contiguous_tree/base.h"
#include "emlabcpp/experimental/testing/base.h"
#include "emlabcpp/protocol.h"
#include "emlabcpp/protocol/packet.h"

#pragma once

namespace emlabcpp::testing
{

enum testing_messages_enum : uint8_t
{
        TESTING_EXEC              = 0x1,
        TESTING_COUNT             = 0x2,
        TESTING_NAME              = 0x3,
        TESTING_LOAD              = 0x4,
        TESTING_SUITE_NAME        = 0x6,
        TESTING_SUITE_DATE        = 0x7,
        TESTING_COLLECT           = 0x8,
        TESTING_FINISHED          = 0x9,
        TESTING_ERROR             = 0xa,
        TESTING_FAILURE           = 0xb,
        TESTING_PARAM_VALUE       = 0x10,
        TESTING_PARAM_CHILD       = 0x11,
        TESTING_PARAM_CHILD_COUNT = 0x12,
        TESTING_PARAM_KEY         = 0x13,
        TESTING_PARAM_TYPE        = 0x14,
        TESTING_INTERNAL_ERROR    = 0xf0,
        TESTING_PROTOCOL_ERROR    = 0xf1,
        TESTING_TREE_ERROR        = 0xf2,
};

template < testing_messages_enum ID >
struct testing_get_property
{
        static constexpr auto tag = ID;
};

struct testing_get_test_name
{
        static constexpr auto tag = TESTING_NAME;
        testing_test_id       tid;
};

struct testing_load_test
{
        static constexpr auto tag = TESTING_LOAD;
        testing_test_id       tid;
        run_id        rid;
};

struct testing_collect_reply
{
        static constexpr auto tag = TESTING_COLLECT;
        run_id        rid;
        testing_node_id       nid;
};

struct testing_param_value_reply
{
        static constexpr auto tag = TESTING_PARAM_VALUE;
        run_id        rid;
        testing_value         value;
};

struct testing_param_type_reply
{
        static constexpr auto tag = TESTING_PARAM_TYPE;
        run_id        rid;
        testing_node_type     type;
};

struct testing_param_child_reply
{
        static constexpr auto tag = TESTING_PARAM_CHILD;
        run_id        rid;
        testing_node_id       chid;
};

struct testing_param_child_count_reply
{
        static constexpr auto tag = TESTING_PARAM_CHILD_COUNT;
        run_id        rid;
        testing_child_count   count;
};

struct testing_param_key_reply
{
        static constexpr auto tag = TESTING_PARAM_KEY;
        run_id        rid;
        testing_key           key;
};

struct testing_tree_error_reply
{
        static constexpr auto                  tag = TESTING_TREE_ERROR;
        run_id                         rid;
        contiguous_request_adapter_errors_enum err;
        testing_node_id                        nid;
};

struct testing_exec
{
        static constexpr auto tag = TESTING_EXEC;
        run_id        rid;
};

using testing_controller_reactor_group = protocol::tag_group<
    testing_get_property< TESTING_SUITE_NAME >,
    testing_get_property< TESTING_SUITE_DATE >,
    testing_get_property< TESTING_COUNT >,
    testing_get_test_name,
    testing_load_test,
    testing_collect_reply,
    testing_param_value_reply,
    testing_param_child_reply,
    testing_param_child_count_reply,
    testing_param_key_reply,
    testing_param_type_reply,
    testing_tree_error_reply,
    testing_exec >;

using testing_controller_reactor_variant = typename testing_controller_reactor_group::value_type;

enum testing_error_enum : uint8_t
{
        TESTING_TEST_NOT_LOADED_E     = 0x1,
        TESTING_TEST_NOT_FOUND_E      = 0x2,
        TESTING_WRONG_RUN_ID_E        = 0x3,
        TESTING_TEST_ALREADY_LOADED_E = 0x4,
        TESTING_BAD_TEST_ID_E         = 0x5,
        TESTING_UNDESIRED_MSG_E       = 0x6,
        TESTING_NO_RESPONSE_E         = 0x7,
        TESTING_TREE_E                = 0x8,
        TESTING_WRONG_TYPE_E          = 0x9,
        TESTING_WRONG_MESSAGE_E       = 0xa
};

struct testing_reactor_error_group  //
  : protocol::command_group<
        protocol::PROTOCOL_BIG_ENDIAN,
        protocol::command< TESTING_TEST_NOT_LOADED_E >,
        protocol::command< TESTING_TEST_NOT_FOUND_E >,
        protocol::command< TESTING_WRONG_RUN_ID_E >,
        protocol::command< TESTING_TEST_ALREADY_LOADED_E >,
        protocol::command< TESTING_BAD_TEST_ID_E >,
        protocol::command< TESTING_UNDESIRED_MSG_E >,
        protocol::command< TESTING_NO_RESPONSE_E >::with_args< testing_messages_enum >,
        protocol::command<
            TESTING_TREE_E >::with_args< testing_node_id, contiguous_request_adapter_errors_enum >,
        protocol::command< TESTING_WRONG_TYPE_E >::with_args< testing_node_id >,
        protocol::command< TESTING_WRONG_MESSAGE_E >

        >
{
};

using testing_reactor_error_variant = typename testing_reactor_error_group::value_type;

struct testing_reactor_controller_group
  : protocol::command_group<
        protocol::PROTOCOL_BIG_ENDIAN,
        protocol::command< TESTING_COUNT >::with_args< testing_test_id >,
        protocol::command< TESTING_NAME >::with_args< testing_name_buffer >,
        protocol::command< TESTING_PARAM_VALUE >::with_args< run_id, testing_node_id >,
        protocol::command< TESTING_PARAM_CHILD >::with_args<
            run_id,
            testing_node_id,
            std::variant< testing_key, testing_child_id > >,
        protocol::command<
            TESTING_PARAM_CHILD_COUNT >::with_args< run_id, testing_node_id >,
        protocol::command< TESTING_PARAM_KEY >::with_args<  //
            run_id,
            testing_node_id,
            testing_child_id >,
        protocol::command< TESTING_PARAM_TYPE >::with_args< run_id, testing_node_id >,
        protocol::command< TESTING_COLLECT >::with_args<
            run_id,
            testing_node_id,
            std::optional< testing_key >,
            testing_collect_arg >,
        protocol::command< TESTING_FINISHED >::with_args< run_id >,
        protocol::command< TESTING_ERROR >::with_args< run_id >,
        protocol::command< TESTING_FAILURE >::with_args< run_id >,
        protocol::command< TESTING_SUITE_NAME >::with_args< testing_name_buffer >,
        protocol::command< TESTING_SUITE_DATE >::with_args< testing_name_buffer >,
        protocol::command< TESTING_INTERNAL_ERROR >::with_args< testing_reactor_error_group >,
        protocol::command< TESTING_PROTOCOL_ERROR >::with_args< protocol::error_record > >
{
};

using testing_reactor_controller_variant = typename testing_reactor_controller_group::value_type;

struct testing_packet_def
{
        static constexpr protocol::endianess_enum endianess = protocol::PROTOCOL_BIG_ENDIAN;
        static constexpr std::array< uint8_t, 4 > prefix    = { 0x42, 0x42, 0x42, 0x42 };
        using size_type                                     = uint16_t;
        using checksum_type                                 = uint8_t;

        static constexpr checksum_type get_checksum( const view< const uint8_t* > msg )
        {
                uint8_t init = 0x0;
                return accumulate( msg, init, [&]( uint8_t accum, uint8_t val ) -> uint8_t {
                        return accum ^ val;
                } );
        }
};

using testing_reactor_controller_packet =
    protocol::packet< testing_packet_def, testing_reactor_controller_group >;
using testing_controller_reactor_packet =
    protocol::packet< testing_packet_def, testing_controller_reactor_group >;

using testing_reactor_controller_msg = typename testing_reactor_controller_packet::message_type;
using testing_controller_reactor_msg = typename testing_controller_reactor_packet::message_type;

testing_reactor_controller_msg
testing_reactor_controller_serialize( const testing_reactor_controller_variant& );
either< testing_reactor_controller_variant, protocol::error_record >
testing_reactor_controller_extract( const testing_reactor_controller_msg& );

testing_controller_reactor_msg
testing_controller_reactor_serialize( const testing_controller_reactor_variant& );
either< testing_controller_reactor_variant, protocol::error_record >
testing_controller_reactor_extract( const testing_controller_reactor_msg& );

}  // namespace emlabcpp::testing
