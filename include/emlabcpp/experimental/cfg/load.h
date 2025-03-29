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

template < typename T, std::endian Endianess, typename ChecksumFunction >
std::tuple< bool, T, std::span< std::byte > >
load_impl( std::span< std::byte > buffer, ChecksumFunction&& chcksm_f )
{
        using sig_conv = protocol::converter_for< checksum, Endianess >;
        using conv     = protocol::converter_for< T, Endianess >;

        T result{};

        checksum                          chcksm;
        protocol::conversion_result const sres = sig_conv::deserialize( buffer, chcksm );
        if ( sres.has_error() )
                return { false, result, buffer };

        protocol::conversion_result const cres =
            conv::deserialize( buffer.subspan( sig_conv::max_size ), result );

        std::span< std::byte > const data = buffer.subspan( sig_conv::max_size, cres.used );

        bool const chcksum_matches = chcksm_f( data ) == chcksm;
        return {
            !cres.has_error() && chcksum_matches,
            result,
            buffer.subspan( sig_conv::max_size + cres.used ) };
}

}  // namespace emlabcpp::cfg
