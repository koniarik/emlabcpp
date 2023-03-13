///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
/// associated documentation files (the "Software"), to deal in the Software without restriction,
/// including without limitation the rights to use, copy, modify, merge, publish, distribute,
/// sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or substantial
/// portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
/// NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
/// OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
/// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#include <algorithm>
#include <array>

#pragma once

namespace emlabcpp::protocol
{

/// Error handling in protocol works with marks. Mark is a string made of eight characters that
/// represents label. Each error is labeled by namespace specific for the source project and by name
/// unique in that namespace, part of the error is also index of byte that caused the problem. These
/// can be easily sent with the protocol lib itself.

static constexpr std::size_t mark_size = 16;

struct mark : std::array< char, mark_size >
{
        using base_type = std::array< char, mark_size >;
};

struct error_record
{
        mark        error_mark;
        std::size_t offset;
};

/// Creates mark from simple string literal.
/// NOLINTNEXTLINE(modernize-avoid-c-arrays)
constexpr mark make_mark( const char ( &msg )[mark_size + 1] )
{
        /// note: do not try to fix the argument type, this is correct approach.
        mark res;
        std::copy_n( msg, res.size(), res.begin() );
        return res;
}

static constexpr auto SIZE_ERR = make_mark( "EMLABCPPSIZE    " );
/// not enough bytes left in the message for the item
static constexpr auto LOWSIZE_ERR = make_mark( "EMLABCPPLOWSIZE " );
/// too much bytes left in the message for the item
static constexpr auto BIGSIZE_ERR = make_mark( "EMLABCPPBIGSIZE " );
/// value in the message is outside of the range of bounded type
static constexpr auto BOUNDS_ERR = make_mark( "EMLABCPPBOUNDS  " );
/// variant id is outside of the range for defined variant
static constexpr auto UNDEFVAR_ERR = make_mark( "EMLABCPPUNDEFVAR" );
/// parsed value is not correct, such as constant
static constexpr auto BADVAL_ERR = make_mark( "EMLABCPPBADVAL  " );
/// no item of group matched the content of message
static constexpr auto GROUP_ERR = make_mark( "EMLABCPPGRPMATCH" );
/// wrong checksum in the protocol
static constexpr auto CHECKSUM_ERR = make_mark( "EMLABCPPCHECKSUM" );

}  // namespace emlabcpp::protocol
