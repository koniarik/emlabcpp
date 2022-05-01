#include "emlabcpp/algorithm.h"
#include "emlabcpp/experimental/testing/base.h"
#include "emlabcpp/protocol.h"
#include "emlabcpp/protocol/packet.h"

#pragma once

namespace emlabcpp
{

enum testing_messages_enum : uint8_t
{
        TESTING_EXEC           = 0x1,
        TESTING_COUNT          = 0x2,
        TESTING_NAME           = 0x3,
        TESTING_LOAD           = 0x4,
        TESTING_ARG            = 0x5,
        TESTING_SUITE_NAME     = 0x6,
        TESTING_SUITE_DATE     = 0x7,
        TESTING_COLLECT        = 0x8,
        TESTING_FINISHED       = 0x9,
        TESTING_ERROR          = 0xa,
        TESTING_FAILURE        = 0xb,
        TESTING_INTERNAL_ERROR = 0xf0,
        TESTING_PROTOCOL_ERROR = 0xf1,
        TESTING_ARG_MISSING    = 0xf2
};

struct testing_controller_reactor_group
  : protocol_command_group<
        PROTOCOL_BIG_ENDIAN,
        protocol_command< TESTING_SUITE_NAME >,
        protocol_command< TESTING_SUITE_DATE >,
        protocol_command< TESTING_COUNT >,
        protocol_command< TESTING_NAME >::with_args< testing_test_id >,
        protocol_command< TESTING_LOAD >::with_args< testing_test_id, testing_run_id >,
        protocol_command<
            TESTING_ARG >::with_args< testing_run_id, testing_key, testing_arg_variant >,
        protocol_command< TESTING_ARG_MISSING >::with_args< testing_run_id, testing_key >,
        protocol_command< TESTING_EXEC >::with_args< testing_run_id > >
{
};

using testing_controller_reactor_variant = typename testing_controller_reactor_group::value_type;

enum testing_error_enum : uint8_t
{
        TESTING_TEST_NOT_LOADED_E     = 1,
        TESTING_TEST_NOT_FOUND_E      = 2,
        TESTING_WRONG_RUN_ID_E        = 3,
        TESTING_TEST_ALREADY_LOADED_E = 4,
        TESTING_BAD_TEST_ID_E         = 5,
        TESTING_UNDESIRED_MSG_E       = 6,
        TESTING_NO_RESPONSE_E         = 7,
        TESTING_ARG_MISSING_E         = 8,
        TESTING_ARG_WRONG_MESSAGE_E   = 9,
        TESTING_ARG_WRONG_TYPE_E      = 10
};

struct testing_reactor_error_group  //
  : protocol_command_group<
        PROTOCOL_BIG_ENDIAN,
        protocol_command< TESTING_TEST_NOT_LOADED_E >,
        protocol_command< TESTING_TEST_NOT_FOUND_E >,
        protocol_command< TESTING_WRONG_RUN_ID_E >,
        protocol_command< TESTING_TEST_ALREADY_LOADED_E >,
        protocol_command< TESTING_BAD_TEST_ID_E >,
        protocol_command< TESTING_UNDESIRED_MSG_E >,
        protocol_command< TESTING_NO_RESPONSE_E >::with_args< testing_messages_enum >,
        protocol_command< TESTING_ARG_MISSING_E >::with_args< testing_key >,
        protocol_command< TESTING_ARG_WRONG_MESSAGE_E >::with_args< testing_messages_enum >,
        protocol_command< TESTING_ARG_WRONG_TYPE_E >::with_args< testing_key > >
{
};

using testing_reactor_error_variant = typename testing_reactor_error_group::value_type;

struct testing_reactor_controller_group
  : protocol_command_group<
        PROTOCOL_BIG_ENDIAN,
        protocol_command< TESTING_COUNT >::with_args< testing_test_id >,
        protocol_command< TESTING_NAME >::with_args< testing_name_buffer >,
        protocol_command< TESTING_ARG >::with_args< testing_run_id, testing_key >,
        protocol_command<
            TESTING_COLLECT >::with_args< testing_run_id, testing_key, testing_arg_variant >,
        protocol_command< TESTING_FINISHED >::with_args< testing_run_id >,
        protocol_command< TESTING_ERROR >::with_args< testing_run_id >,
        protocol_command< TESTING_FAILURE >::with_args< testing_run_id >,
        protocol_command< TESTING_SUITE_NAME >::with_args< testing_name_buffer >,
        protocol_command< TESTING_SUITE_DATE >::with_args< testing_name_buffer >,
        protocol_command< TESTING_INTERNAL_ERROR >::with_args< testing_reactor_error_group >,
        protocol_command< TESTING_PROTOCOL_ERROR >::with_args< protocol_error_record > >
{
};

using testing_reactor_controller_variant = typename testing_reactor_controller_group::value_type;

struct testing_packet_def
{
        static constexpr protocol_endianess_enum  endianess = PROTOCOL_BIG_ENDIAN;
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
    protocol_packet< testing_packet_def, testing_reactor_controller_group >;
using testing_controller_reactor_packet =
    protocol_packet< testing_packet_def, testing_controller_reactor_group >;

using testing_reactor_controller_msg = typename testing_reactor_controller_packet::message_type;
using testing_controller_reactor_msg = typename testing_controller_reactor_packet::message_type;

}  /// namespace emlabcpp
