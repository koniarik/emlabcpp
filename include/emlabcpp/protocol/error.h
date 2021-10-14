#include <array>

#pragma once

namespace emlabcpp
{

// Error handling in protocol works with marks. Mark is a string made of eight characters that
// represents label. Each error is labeled by namespace specific for the source project and by name
// unique in that namespace, part of the error is also index of byte that caused the problem. These
// can be easily sent with the protocol lib itself.

struct protocol_mark : std::array< char, 8 >
{
};

struct protocol_error_record
{
        protocol_mark ns;
        protocol_mark err;
        std::size_t   byte_index;

        friend constexpr bool
        operator==( const protocol_error_record&, const protocol_error_record& ) = default;
};

// Creates protocol_mark from simple string literal.
inline constexpr protocol_mark make_protocol_mark( const char ( &msg )[9] )
{
        // note: do not try to fix the argument type, this is correct approach.
        protocol_mark res;
        std::copy_n( msg, res.size(), res.begin() );
        return res;
}

// error namespace for protocol used by the protocol library by itself.
static constexpr auto PROTOCOL_NS = make_protocol_mark( "EMLABPRO" );

// not enough bytes left in the message for the item
static constexpr auto LOWSIZE_ERR = make_protocol_mark( "LOWSIZE " );
// too much bytes left in the message for the item
static constexpr auto BIGSIZE_ERR = make_protocol_mark( "BIGSIZE " );
// value in the message is outside of the range of bounded type
static constexpr auto BOUNDS_ERR = make_protocol_mark( "BOUNDS  " );
// variant id is outside of the range for defined variant
static constexpr auto UNDEFVAR_ERR = make_protocol_mark( "UNDEFVAR" );
// parsed value is not correct, such as constant
static constexpr auto BADVAL_ERR = make_protocol_mark( "BADVAL  " );
// no item of group matched the content of message
static constexpr auto GROUP_ERR = make_protocol_mark( "GRPMATCH" );

}  // namespace emlabcpp
