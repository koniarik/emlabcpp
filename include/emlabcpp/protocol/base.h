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

#include "emlabcpp/bounded.h"
#include "emlabcpp/protocol/error.h"

#include <bit>
#include <bitset>
#include <cstring>
#include <type_traits>
#include <variant>

namespace emlabcpp::protocol
{

struct conversion_result
{

        std::size_t used  = 0;
        const mark* error = nullptr;

        constexpr conversion_result() = default;

        explicit constexpr conversion_result( const std::size_t u )
          : used( u )
        {
        }

        constexpr conversion_result( const std::size_t u, const mark* const m )
          : used( u )
          , error( m )
        {
        }

        [[nodiscard]] constexpr bool has_error() const
        {
                return error != nullptr;
        }

        [[nodiscard]] constexpr const mark* get_error() const
        {
                return error;
        }
};

/// Concept that matches types considered base - serialized directly by using byte shifting.
template < typename T >
concept base_type = std::is_floating_point_v< T > || std::is_integral_v< T > || std::is_enum_v< T >;

/// Follows a set of special data types used for definition of protocol. These either represent
/// special types or affect the serialization/deserialization process of normal types.
/// -----------------------------------------------------------------------------------------------

/// Changes the endianess of definition D.
template < std::endian Endianess, typename D >
struct endianess_wrapper
{
        static constexpr std::endian value = Endianess;
        using def_type                     = D;
};

/// Serializes values from definitions Ds to std::variant. The byte message does not contain
/// identificator of variant used, rather the first definition that manages to deserialize the
/// message is used.
template < typename... Ds >
struct group
{
        using def_type = std::variant< Ds... >;
};

template < typename... Ds >
struct tag_group
{
        using def_type = std::variant< Ds... >;
};

/// Creates a segment starting with counter defined by CounterDef, this counter limits how many
/// bytes are passed to deserialization process, bytes after the limit ale not considered by this
/// segment.
template < typename CounterDef, typename D >
struct sized_buffer
{
        using counter_type = CounterDef;
        using def_type     = D;
};

/// The value defined by `D` present in the message is offseted by `Offset`. If the offset for
/// example `2`, value `4` in the message is parsed as `2` and value `1` is serialized as `3`.
template < typename D, auto Offset >
struct value_offset
{
        static constexpr auto offset = Offset;
        using def_type               = D;
};

/// More complex constructs have custom mechanics that internally produces `def_type` alias used by
/// the library to serialize/deserialize it. Type inheriting htis class are handled as their
/// `def_type`.
struct converter_def_type_base
{
};

}  // namespace emlabcpp::protocol
