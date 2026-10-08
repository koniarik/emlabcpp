/// MIT License
///
/// Copyright (c) 2025-2026 Jan Veverak Koniarik
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

#include "../experimental/string_buffer.hpp"

#include <algorithm>
#include <array>

namespace emlabcpp::protocol
{

static constexpr std::size_t mark_size = 16;

using mark = string_buffer< mark_size >;

struct error_record
{
        mark        error_mark;
        std::size_t offset;
};

static constexpr auto size_err = mark( "EMCPPSIZE" );
/// not enough bytes left in the message for the item
static constexpr auto lowsize_err = mark( "EMCPPLOWSIZE" );
/// too much bytes left in the message for the item
static constexpr auto bigsize_err = mark( "EMCPPBIGSIZE" );
/// value in the message is outside of the range of bounded type
static constexpr auto bounds_err = mark( "EMCPPBOUNDS" );
/// variant id is outside of the range for defined variant
static constexpr auto undefvar_err = mark( "EMCPPUNDEFVAR" );
/// parsed value is not correct, such as constant
static constexpr auto badval_err = mark( "EMCPPBADVAL" );
/// no item of group matched the content of message
static constexpr auto group_err = mark( "EMCPPGRPMTCH" );
/// wrong checksum in the protocol
static constexpr auto checksum_err = mark( "EMCPPCHECKSM" );

}  // namespace emlabcpp::protocol
