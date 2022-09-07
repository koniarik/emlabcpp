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
#include "emlabcpp/bounded.h"
#include "emlabcpp/protocol/error.h"

#include <bit>
#include <bitset>
#include <cstring>
#include <type_traits>
#include <variant>

#pragma once

namespace emlabcpp::protocol
{

enum class error_possibility
{
        POSSIBLE,
        IMPOSSIBLE
};

/// Strucutre used as result of deserialization in the internal mechanisms of protocol handling.
/// Contains parsed value and value of how much bytes were used.
template < typename T, error_possibility CanErr = error_possibility::POSSIBLE >
struct conversion_result;

template < typename T >
struct conversion_result< T, error_possibility::IMPOSSIBLE >;

template < typename T, error_possibility CanErr >
struct conversion_result
{
        static constexpr error_possibility can_err = CanErr;

        std::size_t                    used = 0;
        std::variant< T, const mark* > res;

        conversion_result() = default;
        conversion_result( const std::size_t u, const std::variant< T, const mark* >& v )
          : used( u )
          , res( v )
        {
        }
        conversion_result( const std::size_t u, const T& v )
          : used( u )
          , res( v )
        {
        }
        conversion_result( const std::size_t u, const mark* m )
          : used( u )
          , res( m )
        {
        }

        explicit conversion_result(
            const conversion_result< T, error_possibility::IMPOSSIBLE >& other )
          : conversion_result( other.used, other.res )
        {
        }

        [[nodiscard]] bool has_error() const
        {
                return std::holds_alternative< const mark* >( res );
        }

        [[nodiscard]] const mark* get_error() const
        {
                if ( has_error() ) {
                        return *std::get_if< const mark* >( &res );
                }
                return nullptr;
        }

        [[nodiscard]] const T* get_value() const
        {
                return std::get_if< T >( &res );
        }

        template < typename UnaryCallable >
        auto convert_value( const UnaryCallable& cb ) &&  //
            -> conversion_result< decltype( cb( std::declval< T >() ) ) >
        {
                if ( std::holds_alternative< const mark* >( res ) ) {
                        return { used, *std::get_if< const mark* >( &res ) };
                } else {
                        return { used, cb( std::move( *std::get_if< T >( &res ) ) ) };
                }
        }

        template < typename BinaryCallable >
        auto
        bind_value( const BinaryCallable& cb ) && -> decltype( cb( used, std::declval< T >() ) )
        {
                if ( std::holds_alternative< const mark* >( res ) ) {
                        return { used, *std::get_if< const mark* >( &res ) };
                } else {
                        return cb( used, std::move( *std::get_if< T >( &res ) ) );
                }
        }
};

template < typename T >
struct conversion_result< T, error_possibility::IMPOSSIBLE >
{
        static constexpr error_possibility can_err = error_possibility::IMPOSSIBLE;

        std::size_t             used = 0;
        [[no_unique_address]] T res;

        conversion_result() = default;
        conversion_result( const std::size_t u, const T& v )
          : used( u )
          , res( v )
        {
        }

        [[nodiscard]] const T* get_value() const
        {
                return &res;
        }

        template < typename UnaryCallable >
        auto convert_value( const UnaryCallable& cb ) &&  //
            -> conversion_result<
                decltype( cb( std::declval< T >() ) ),
                error_possibility::IMPOSSIBLE >
        {
                return { used, cb( std::move( res ) ) };
        }

        template < typename BinaryCallable >
        auto
        bind_value( const BinaryCallable& cb ) && -> decltype( cb( used, std::declval< T >() ) )
        {
                return cb( used, std::move( res ) );
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
