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
#pragma once

#include "../../protocol/converter.h"
#include "./base.h"

#include <span>
#include <tuple>

namespace emlabcpp::cfg
{

template < std::endian Endianess, typename T, typename ChecksumFunction >
std::tuple< bool, std::span< std::byte > >
store_impl( std::span< std::byte > buffer, T const& item, ChecksumFunction&& chcksm_f )
{
        using sig_conv = protocol::converter_for< checksum, Endianess >;
        using conv     = protocol::converter_for< T, Endianess >;
        if ( buffer.size() < sig_conv::max_size + conv::max_size )
                return { false, buffer };
        std::span< std::byte > data = buffer.subspan< sig_conv::max_size, conv::max_size >();
        bounded const used  = conv::serialize_at( data.subspan< 0, conv::max_size >(), item );
        data                = data.first( *used );
        checksum const chck = chcksm_f( data );
        sig_conv::serialize_at( buffer.subspan< 0, sig_conv::max_size >(), chck );
        return { true, buffer.subspan( sig_conv::max_size + data.size() ) };
}

}  // namespace emlabcpp::cfg
