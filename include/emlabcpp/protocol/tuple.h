// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//
//  Copyright © 2022 Jan Veverak Koniarik
//  This file is part of project: emlabcpp
//
#include "emlabcpp/protocol/traits.h"

#pragma once

namespace emlabcpp::protocol
{

/// protocol_tuple is high levle alternative to use just 'std::tuple' that is more friendly for
/// standalone protocols. It is designed for message that are simply a set of serialized items. It
/// also provieds more readable syntax. The template arguments at first configurate the protocol and
/// later is follow by items present in the tuple, this can also be added with
/// protocol_tuple::with_items alias that appends the items. For example:
//
/// protocol_tuple< PROTOCOL_BIG_ENDIAN >::with_items< uint32_t, uint32_t >;
//
/// serializes/deserializes in same way as 'std::tuple<uint32_t,uint32_t>' configured for big
/// endianess.
template < endianess_enum Endianess, typename... Ds >
struct protocol_tuple : converter_def_type_base
{
        template < typename... SubDs >
        using with_items = protocol_tuple< Endianess, Ds..., SubDs... >;

        using def_type = protocol_endianess< Endianess, std::tuple< Ds... > >;
        using decl     = proto_traits< def_type >;

        static constexpr std::size_t max_size = decl::max_size;

        using value_type   = typename decl::value_type;
        using message_type = message< max_size >;

        constexpr static value_type
        make_val( const typename proto_traits< Ds >::value_type&... args )
        {
                return value_type{ args... };
        }
};

}  // namespace emlabcpp::protocol
