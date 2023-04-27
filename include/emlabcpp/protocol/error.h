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

#include "emlabcpp/experimental/string_buffer.h"

#include <algorithm>
#include <array>

#pragma once

namespace emlabcpp::protocol
{

static constexpr std::size_t mark_size = 16;

using mark = string_buffer< mark_size >;

struct error_record
{
        mark        error_mark;
        std::size_t offset;
};

static constexpr auto SIZE_ERR = mark( "EMCPPSIZE" );
/// not enough bytes left in the message for the item
static constexpr auto LOWSIZE_ERR = mark( "EMCPPLOWSIZE" );
/// too much bytes left in the message for the item
static constexpr auto BIGSIZE_ERR = mark( "EMCPPBIGSIZE" );
/// value in the message is outside of the range of bounded type
static constexpr auto BOUNDS_ERR = mark( "EMCPPBOUNDS" );
/// variant id is outside of the range for defined variant
static constexpr auto UNDEFVAR_ERR = mark( "EMCPPUNDEFVAR" );
/// parsed value is not correct, such as constant
static constexpr auto BADVAL_ERR = mark( "EMCPPBADVAL" );
/// no item of group matched the content of message
static constexpr auto GROUP_ERR = mark( "EMCPPGRPMTCH" );
/// wrong checksum in the protocol
static constexpr auto CHECKSUM_ERR = mark( "EMCPPCHECKSM" );

}  // namespace emlabcpp::protocol
