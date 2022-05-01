#include <algorithm>
#include <array>

#pragma once

namespace emlabcpp
{

/// Error handling in protocol works with marks. Mark is a string made of eight characters that
/// represents label. Each error is labeled by namespace specific for the source project and by name
/// unique in that namespace, part of the error is also index of byte that caused the problem. These
/// can be easily sent with the protocol lib itself.

struct protocol_mark : std::array< char, 16 >
{
        using base_type = std::array< char, 16 >;
};

struct protocol_error_record
{
        protocol_mark mark;
        std::size_t   offset;
};

/// Creates protocol_mark from simple string literal.
/// NOLINTNEXTLINE(modernize-avoid-c-arrays)
inline constexpr protocol_mark make_protocol_mark( const char ( &msg )[17] )
{
        /// note: do not try to fix the argument type, this is correct approach.
        protocol_mark res;
        std::copy_n( msg, res.size(), res.begin() );
        return res;
}

static constexpr auto SIZE_ERR = make_protocol_mark( "EMLABCPPSIZE    " );
/// not enough bytes left in the message for the item
static constexpr auto LOWSIZE_ERR = make_protocol_mark( "EMLABCPPLOWSIZE " );
/// too much bytes left in the message for the item
static constexpr auto BIGSIZE_ERR = make_protocol_mark( "EMLABCPPBIGSIZE " );
/// value in the message is outside of the range of bounded type
static constexpr auto BOUNDS_ERR = make_protocol_mark( "EMLABCPPBOUNDS  " );
/// variant id is outside of the range for defined variant
static constexpr auto UNDEFVAR_ERR = make_protocol_mark( "EMLABCPPUNDEFVAR" );
/// parsed value is not correct, such as constant
static constexpr auto BADVAL_ERR = make_protocol_mark( "EMLABCPPBADVAL  " );
/// no item of group matched the content of message
static constexpr auto GROUP_ERR = make_protocol_mark( "EMLABCPPGRPMATCH" );
/// wrong checksum in the protocol
static constexpr auto CHECKSUM_ERR = make_protocol_mark( "EMLABCPPCHECKSUM" );

}  /// namespace emlabcpp
