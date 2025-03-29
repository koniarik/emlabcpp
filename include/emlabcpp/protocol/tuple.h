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

#include "./traits.h"

namespace emlabcpp::protocol
{

/// tuple is high levle alternative to use just 'std::tuple' that is more friendly for
/// standalone protocols. It is designed for message that are simply a set of serialized items. It
/// also provieds more readable syntax. The template arguments at first configurate the protocol and
/// later is follow by items present in the tuple, this can also be added with
/// tuple::with_items alias that appends the items. For example:
//
/// tuple< BIG_ENDIAN >::with_items< uint32_t, uint32_t >;
//
/// serializes/deserializes in same way as 'std::tuple<uint32_t,uint32_t>' configured for big
/// endianess.
template < std::endian Endianess, typename... Ds >
struct tuple : converter_def_type_base
{
        template < typename... SubDs >
        using with_items = tuple< Endianess, Ds..., SubDs... >;

        using def_tuple = std::tuple< Ds... >;

        template < std::size_t N >
        using nth_def = std::tuple_element_t< N, def_tuple >;

        using def_type = endianess_wrapper< Endianess, def_tuple >;
        using traits   = proto_traits< def_type >;

        static constexpr std::size_t max_size = traits::max_size;

        using value_type   = typename traits::value_type;
        using message_type = message< max_size >;

        constexpr static value_type
        make_val( typename proto_traits< Ds >::value_type const&... args )
        {
                return value_type{ args... };
        }
};

}  // namespace emlabcpp::protocol
