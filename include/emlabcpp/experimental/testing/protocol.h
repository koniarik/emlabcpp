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
        static constexpr auto id = ID;
};

struct get_count_reply
{
        static constexpr auto id = COUNT;
        test_id               count;
};

struct get_suite_name_reply
{
        static constexpr auto id = SUITE_NAME;
        name_buffer           name;
};

struct get_suite_date_reply
{
        static constexpr auto id = SUITE_DATE;
        name_buffer           date;
};

struct get_test_name_request
{
        static constexpr auto id = NAME;
        test_id               tid;
};

struct get_test_name_reply
{
        static constexpr auto id = NAME;
        name_buffer           name;
};

struct load_test
{
        static constexpr auto id = LOAD;
        test_id               tid;
        run_id                rid;
};

struct collect_request
{
        static constexpr auto     id = COLLECT;
        run_id                    rid;
        node_id                   parent;
        std::optional< key_type > opt_key;
        collect_value_type        value;
};

struct collect_reply
{
        static constexpr auto id = COLLECT;
        run_id                rid;
        node_id               nid;
};

struct param_value_request
{
        static constexpr auto id = PARAM_VALUE;
        run_id                rid;
        node_id               nid;
};

struct param_value_reply
{
        static constexpr auto id = PARAM_VALUE;
        run_id                rid;
        value_type            value;
};

struct param_child_request
{
        static constexpr auto              id = PARAM_CHILD;
        run_id                             rid;
        node_id                            parent;
        std::variant< key_type, child_id > chid;
};

struct param_child_reply
{
        static constexpr auto id = PARAM_CHILD;
        run_id                rid;
        node_id               chid;
};

struct param_child_count_request
{
        static constexpr auto id = PARAM_CHILD_COUNT;
        run_id                rid;
        node_id               parent;
};

struct param_child_count_reply
{
        static constexpr auto id = PARAM_CHILD_COUNT;
        run_id                rid;
        child_count           count;
};

struct param_key_request
{
        static constexpr auto id = PARAM_KEY;
        run_id                rid;
        node_id               nid;
        child_id              chid;
};

struct param_key_reply
{
        static constexpr auto id = PARAM_KEY;
        run_id                rid;
        key_type              key;
};

struct param_type_request
{
        static constexpr auto id = PARAM_TYPE;
        run_id                rid;
        node_id               nid;
};

struct param_type_reply
{
        static constexpr auto id = PARAM_TYPE;
        run_id                rid;
        node_type_enum        type;
};

struct tree_error_reply
{
        static constexpr auto                  id = TREE_ERROR;
        run_id                                 rid;
        contiguous_request_adapter_errors_enum err;
        node_id                                nid;
};

struct test_finished
{
        static constexpr auto id = FINISHED;
        run_id                rid;
        bool                  errored;
        bool                  failed;
};

struct exec_request
{
        static constexpr auto id = EXEC;
        run_id                rid;
};

struct reactor_protocol_error_report
{
        static constexpr auto  id = PROTOCOL_ERROR;
        protocol::error_record rec;
};

using controller_reactor_group = protocol::tag_group<
    get_property< SUITE_NAME >,
    get_property< SUITE_DATE >,
    get_property< COUNT >,
    get_test_name_request,
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

template < error_enum Err >
struct error
{
        static constexpr auto id = Err;
};

struct no_response_error
{
        static constexpr auto id = NO_RESPONSE_E;
        messages_enum         msg;
};

struct wrong_type_error
{
        static constexpr auto id = WRONG_TYPE_E;
        node_id               nid;
};

// TODO: this is not good, we want group here
using reactor_error_variant = std::variant<
    error< TEST_NOT_LOADED_E >,
    error< TEST_NOT_FOUND_E >,
    error< WRONG_RUN_ID_E >,
    error< TEST_ALREADY_LOADED_E >,
    error< BAD_TEST_ID_E >,
    error< UNDESIRED_MSG_E >,
    no_response_error,
    wrong_type_error,
    tree_error_reply,
    error< WRONG_MESSAGE_E > >;

struct reactor_internal_error_report
{
        static constexpr auto id = INTERNAL_ERROR;
        reactor_error_variant var;
};

using reactor_controller_group = protocol::tag_group<
    get_count_reply,
    get_test_name_reply,
    collect_request,
    param_value_request,
    param_child_request,
    param_child_count_request,
    param_key_request,
    param_type_request,
    test_finished,
    get_suite_name_reply,
    get_suite_date_reply,
    reactor_internal_error_report,
    reactor_protocol_error_report >;

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
