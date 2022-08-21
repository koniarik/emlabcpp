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

enum messages_enum : uint8_t
{
        EXEC              = 0x1,
        COUNT             = 0x2,
        NAME              = 0x3,
        LOAD              = 0x4,
        SUITE_NAME        = 0x6,
        SUITE_DATE        = 0x7,
        COLLECT           = 0x8,
        FINISHED          = 0x9,
        ERROR             = 0xa,
        FAILURE           = 0xb,
        PARAM_VALUE       = 0x10,
        PARAM_CHILD       = 0x11,
        PARAM_CHILD_COUNT = 0x12,
        PARAM_KEY         = 0x13,
        PARAM_TYPE        = 0x14,
        INTERNAL_ERROR    = 0xf0,
        PROTOCOL_ERROR    = 0xf1,
        TREE_ERROR        = 0xf2,
};

template < messages_enum ID >
struct get_property
{
        static constexpr auto tag = ID;
};

struct get_test_name
{
        static constexpr auto tag = NAME;
        test_id               tid;
};

struct load_test
{
        static constexpr auto tag = LOAD;
        test_id               tid;
        run_id                rid;
};

struct collect_reply
{
        static constexpr auto tag = COLLECT;
        run_id                rid;
        node_id               nid;
};

struct param_value_reply
{
        static constexpr auto tag = PARAM_VALUE;
        run_id                rid;
        value_type            value;
};

struct param_type_reply
{
        static constexpr auto tag = PARAM_TYPE;
        run_id                rid;
        node_type_enum     type;
};

struct param_child_reply
{
        static constexpr auto tag = PARAM_CHILD;
        run_id                rid;
        node_id               chid;
};

struct param_child_count_reply
{
        static constexpr auto tag = PARAM_CHILD_COUNT;
        run_id                rid;
        child_count           count;
};

struct param_key_reply
{
        static constexpr auto tag = PARAM_KEY;
        run_id                rid;
        key_type              key;
};

struct tree_error_reply
{
        static constexpr auto                  tag = TREE_ERROR;
        run_id                                 rid;
        contiguous_request_adapter_errors_enum err;
        node_id                                nid;
};

struct exec_request
{
        static constexpr auto tag = EXEC;
        run_id                rid;
};

using controller_reactor_group = protocol::tag_group<
    get_property< SUITE_NAME >,
    get_property< SUITE_DATE >,
    get_property< COUNT >,
    get_test_name,
    load_test,
    collect_reply,
    param_value_reply,
    param_child_reply,
    param_child_count_reply,
    param_key_reply,
    param_type_reply,
    tree_error_reply,
    exec_request >;

using controller_reactor_variant = typename controller_reactor_group::value_type;

enum error_enum : uint8_t
{
        TEST_NOT_LOADED_E     = 0x1,
        TEST_NOT_FOUND_E      = 0x2,
        WRONG_RUN_ID_E        = 0x3,
        TEST_ALREADY_LOADED_E = 0x4,
        BAD_TEST_ID_E         = 0x5,
        UNDESIRED_MSG_E       = 0x6,
        NO_RESPONSE_E         = 0x7,
        TREE_E                = 0x8,
        WRONG_TYPE_E          = 0x9,
        WRONG_MESSAGE_E       = 0xa
};

struct reactor_error_group  //
  : protocol::command_group<
        std::endian::big,
        protocol::command< TEST_NOT_LOADED_E >,
        protocol::command< TEST_NOT_FOUND_E >,
        protocol::command< WRONG_RUN_ID_E >,
        protocol::command< TEST_ALREADY_LOADED_E >,
        protocol::command< BAD_TEST_ID_E >,
        protocol::command< UNDESIRED_MSG_E >,
        protocol::command< NO_RESPONSE_E >::with_args< messages_enum >,
        protocol::command<
            TREE_E >::with_args< node_id, contiguous_request_adapter_errors_enum >,
        protocol::command< WRONG_TYPE_E >::with_args< node_id >,
        protocol::command< WRONG_MESSAGE_E > >
{
};

using reactor_error_variant = typename reactor_error_group::value_type;

struct reactor_controller_group
  : protocol::command_group<
        std::endian::big,
        protocol::command< COUNT >::with_args< test_id >,
        protocol::command< NAME >::with_args< name_buffer >,
        protocol::command< PARAM_VALUE >::with_args< run_id, node_id >,
        protocol::command<
            PARAM_CHILD >::with_args< run_id, node_id, std::variant< key_type, child_id > >,
        protocol::command< PARAM_CHILD_COUNT >::with_args< run_id, node_id >,
        protocol::command< PARAM_KEY >::with_args<  //
            run_id,
            node_id,
            child_id >,
        protocol::command< PARAM_TYPE >::with_args< run_id, node_id >,
        protocol::command< COLLECT >::
            with_args< run_id, node_id, std::optional< key_type >, collect_value_type >,
        protocol::command< FINISHED >::with_args< run_id >,
        protocol::command< ERROR >::with_args< run_id >,
        protocol::command< FAILURE >::with_args< run_id >,
        protocol::command< SUITE_NAME >::with_args< name_buffer >,
        protocol::command< SUITE_DATE >::with_args< name_buffer >,
        protocol::command< INTERNAL_ERROR >::with_args< reactor_error_group >,
        protocol::command< PROTOCOL_ERROR >::with_args< protocol::error_record > >
{
};

using reactor_controller_variant = typename reactor_controller_group::value_type;

struct packet_def
{
        static constexpr std::endian              endianess = std::endian::big;
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

using reactor_controller_packet = protocol::packet< packet_def, reactor_controller_group >;
using controller_reactor_packet = protocol::packet< packet_def, controller_reactor_group >;

using reactor_controller_msg = typename reactor_controller_packet::message_type;
using controller_reactor_msg = typename controller_reactor_packet::message_type;

reactor_controller_msg reactor_controller_serialize( const reactor_controller_variant& );
either< reactor_controller_variant, protocol::error_record >
reactor_controller_extract( const reactor_controller_msg& );

controller_reactor_msg controller_reactor_serialize( const controller_reactor_variant& );
either< controller_reactor_variant, protocol::error_record >
controller_reactor_extract( const controller_reactor_msg& );

}  // namespace emlabcpp::testing
