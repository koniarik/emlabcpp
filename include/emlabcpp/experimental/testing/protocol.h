/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
///

#pragma once

#include "../../algorithm.h"
#include "../../protocol/base.h"
#include "../../protocol/endpoint.h"
#include "../../protocol/error.h"
#include "../../protocol/handler.h"
#include "../../protocol/message.h"
#include "../../protocol/packet.h"
#include "../../protocol/traits.h"
#include "../../static_function.h"
#include "../../view.h"
#include "../contiguous_tree/base.h"
#include "../multiplexer.h"
#include "../testing/base.h"

#include <array>
#include <bit>
#include <variant>

namespace emlabcpp::testing
{

enum class msgid : uint8_t
{
        EXEC           = 0x1,
        COUNT          = 0x2,
        NAME           = 0x3,
        SUITE_NAME     = 0x6,
        SUITE_DATE     = 0x7,
        FINISHED       = 0x9,
        ERROR          = 0xa,
        FAILURE        = 0xb,
        BOOT           = 0xc,
        INTERNAL_ERROR = 0xf0,
        PROTOCOL_ERROR = 0xf1,
        TREE_ERROR     = 0xf2,
};

template < msgid ID >
struct get_property
{
        static constexpr auto id = ID;
};

struct get_count_reply
{
        static constexpr auto id = msgid::COUNT;
        test_id               count;
};

struct get_suite_name_reply
{
        static constexpr auto id = msgid::SUITE_NAME;
        name_buffer           name;
};

struct get_suite_date_reply
{
        static constexpr auto id = msgid::SUITE_DATE;
        name_buffer           date;
};

struct get_test_name_request
{
        static constexpr auto id = msgid::NAME;
        test_id               tid;
};

struct get_test_name_reply
{
        static constexpr auto id = msgid::NAME;
        name_buffer           name;
};

struct tree_error_reply
{
        static constexpr auto             id = msgid::TREE_ERROR;
        contiguous_request_adapter_errors err;
        node_id                           nid;
};

struct test_finished
{
        static constexpr auto id = msgid::FINISHED;
        run_id                rid;
        test_status           status;
};

struct boot
{
        static constexpr auto id = msgid::BOOT;
};

struct exec_request
{
        static constexpr auto id = msgid::EXEC;
        run_id                rid;
        test_id               tid;
};

using controller_reactor_group = protocol::tag_group<
    get_property< msgid::SUITE_NAME >,
    get_property< msgid::SUITE_DATE >,
    get_property< msgid::COUNT >,
    get_test_name_request,
    exec_request >;

using controller_reactor_variant =
    typename protocol::traits_for< controller_reactor_group >::value_type;
using controller_reactor_message =
    typename protocol::handler< controller_reactor_group >::message_type;

enum error_enum : uint8_t
{
        TEST_IS_RUNING_E             = 0x1,
        TEST_NOT_FOUND_E             = 0x2,
        BAD_TEST_ID_E                = 0x5,
        UNDESIRED_MSG_E              = 0x6,
        NO_RESPONSE_E                = 0x7,
        TREE_E                       = 0x8,
        WRONG_TYPE_E                 = 0x9,
        WRONG_MESSAGE_E              = 0xa,
        INPUT_MESSAGE_PROTOCOL_ERROR = 0xb,
};

template < error_enum Err >
struct error
{
        static constexpr auto id = Err;
};

struct no_response_error
{
        static constexpr auto id = NO_RESPONSE_E;
        msgid                 msg;
};

struct wrong_type_error
{
        static constexpr auto id = WRONG_TYPE_E;
        node_id               nid;
};

struct input_message_protocol_error
{
        static constexpr auto  id = INPUT_MESSAGE_PROTOCOL_ERROR;
        protocol::error_record rec;
};

// TODO: this is not good, we want group here
using reactor_error_variant = std::variant<
    error< TEST_IS_RUNING_E >,
    error< TEST_NOT_FOUND_E >,
    error< BAD_TEST_ID_E >,
    error< UNDESIRED_MSG_E >,
    no_response_error,
    wrong_type_error,
    tree_error_reply,
    error< WRONG_MESSAGE_E >,
    input_message_protocol_error >;

struct reactor_internal_error_report
{
        static constexpr auto id = msgid::INTERNAL_ERROR;
        reactor_error_variant var;
};

using reactor_controller_group = protocol::tag_group<
    boot,
    get_count_reply,
    get_test_name_reply,
    test_finished,
    get_suite_name_reply,
    get_suite_date_reply,
    reactor_internal_error_report >;

using reactor_controller_variant =
    typename protocol::traits_for< reactor_controller_group >::value_type;
using reactor_controller_message =
    typename protocol::handler< reactor_controller_group >::message_type;

using reactor_transmit_callback =
    static_function< result( protocol::channel_type, reactor_controller_message const& ), 32 >;
using controller_transmit_callback =
    static_function< result( protocol::channel_type, controller_reactor_message const& ), 32 >;

using packet_payload = protocol::multiplexer_payload< 256 >;
// TODO: this needs rethinking /o\ entire multiplexer needs redesign?

// TODO: this needs more constructive approach... why is it here?
static constexpr protocol::channel_type core_channel = 1;

struct packet_def
{
        static constexpr std::endian                endianess = std::endian::big;
        static constexpr std::array< std::byte, 4 > prefix    = bytes( 0x42, 0x42, 0x42, 0x42 );
        using size_type                                       = uint16_t;
        using checksum_type                                   = std::byte;

        static constexpr checksum_type get_checksum( view< std::byte const* > const msg )
        {
                std::byte const init{ 0x0 };
                return accumulate(
                    msg, init, []( std::byte const accum, std::byte const val ) -> std::byte {
                            return accum ^ val;
                    } );
        }
};

using packet   = protocol::packet< packet_def, packet_payload >;
using endpoint = protocol::multiplexed_endpoint< packet >;
using message  = typename packet::message_type;

}  // namespace emlabcpp::testing

namespace emlabcpp::protocol
{

extern template class endpoint< testing::packet, testing::packet >;

}  // namespace emlabcpp::protocol
