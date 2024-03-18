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

#pragma once

#include "emlabcpp/experimental/decompose.h"
#include "emlabcpp/protocol/base.h"
#include "emlabcpp/protocol/error.h"
#include "emlabcpp/protocol/message.h"
#include "emlabcpp/quantity.h"
#include "emlabcpp/static_vector.h"
#include "emlabcpp/types.h"

#include <optional>
#include <variant>

namespace emlabcpp::protocol
{

/// Each definition of item provided to protocol library should have specialization of
/// 'proto_traits' structure. This contains basic information of how it should be serialized.
template < typename D >
struct proto_traits;

template < typename D >
struct backup_proto_traits;

template < typename D >
consteval auto traits_for_impl()
{
        if constexpr ( with_value_type< proto_traits< D > > )
                return proto_traits< D >{};
        else
                return backup_proto_traits< D >{};
}

template < typename D >
using traits_for = decltype( traits_for_impl< D >() );

/// This concepts limits types to types that can be declared, that is the overload of
/// 'traits_for' is fully defined: traits_for::value_type contains definition of value
/// produced by the declaration, traits_for::max_size contains estimated maximal size in bytes
/// taken by the serialized value in the message. traits_for::min_size should contain minimal
/// size used.
template < typename D >
concept convertible = requires( D val ) {
        { traits_for< D >::max_size } -> std::convertible_to< std::size_t >;
        { traits_for< D >::min_size } -> std::convertible_to< std::size_t >;
        requires std::default_initializable< typename traits_for< D >::value_type >;
};

template < typename T >
concept fixedly_sized = traits_for< T >::min_size == traits_for< T >::max_size;

template < base_type D >
struct proto_traits< D >
{
        using value_type                      = D;
        static constexpr std::size_t max_size = sizeof( D );
        static constexpr std::size_t min_size = max_size;
};

template < convertible D, std::size_t N >
struct proto_traits< std::array< D, N > >
{
        using value_type                      = std::array< D, N >;
        static constexpr std::size_t max_size = traits_for< D >::max_size * N;
        static constexpr std::size_t min_size = traits_for< D >::min_size * N;
};

template < convertible... Ds >
struct proto_traits< std::tuple< Ds... > >
{
        using value_type = std::tuple< typename traits_for< Ds >::value_type... >;
        static constexpr std::size_t max_size = ( traits_for< Ds >::max_size + ... + 0 );
        static constexpr std::size_t min_size = ( traits_for< Ds >::min_size + ... + 0 );
};

template < convertible... Ds >
struct proto_traits< std::variant< Ds... > >
{
        using id_type    = uint8_t;
        using id_traits  = traits_for< id_type >;
        using value_type = std::variant< typename traits_for< Ds >::value_type... >;

        static constexpr std::size_t max_size =
            id_traits::max_size + std::max< std::size_t >( { traits_for< Ds >::max_size... } );
        static constexpr std::size_t min_size =
            id_traits::min_size + std::min< std::size_t >( { traits_for< Ds >::min_size... } );
};

template <>
struct proto_traits< std::monostate >
{
        using value_type                      = std::monostate;
        static constexpr std::size_t max_size = 0;
        static constexpr std::size_t min_size = 0;
};

template < std::size_t N >
struct proto_traits< std::bitset< N > >
{
        using value_type = std::bitset< N >;

        static constexpr std::size_t max_size = ( N + 7 ) / 8;
        static constexpr std::size_t min_size = max_size;
};

template < std::size_t N >
struct proto_traits< sizeless_message< N > >
{
        using value_type = sizeless_message< N >;

        static constexpr std::size_t max_size = N;
        static constexpr std::size_t min_size = 0;
};

template < std::size_t N >
struct proto_traits< message< N > >
{
        using msg_size_type = uint16_t;
        static_assert( N <= std::numeric_limits< msg_size_type >::max() );

        using msg_size_traits = traits_for< msg_size_type >;

        using value_type = message< N >;

        static constexpr std::size_t max_size = msg_size_traits::max_size + N;
        static constexpr std::size_t min_size = msg_size_traits::min_size + 0;
};

template < convertible D, auto Offset >
struct proto_traits< value_offset< D, Offset > >
{
        using def_type   = typename value_offset< D, Offset >::def_type;
        using def_traits = traits_for< def_type >;
        using value_type = typename def_traits::value_type;

        static constexpr std::size_t max_size = def_traits::max_size;
        static constexpr std::size_t min_size = def_traits::min_size;
};

template < quantity_derived D >
struct proto_traits< D >
{
        using value_type = D;

        static constexpr std::size_t max_size = traits_for< typename D::value_type >::max_size;
        static constexpr std::size_t min_size = traits_for< typename D::value_type >::min_size;
};

template < convertible D, D Min, D Max >
struct proto_traits< bounded< D, Min, Max > >
{
        using value_type = bounded< D, Min, Max >;

        static constexpr std::size_t max_size = traits_for< D >::max_size;
        static constexpr std::size_t min_size = traits_for< D >::min_size;
};

template < convertible CounterType, convertible D >
struct proto_traits< sized_buffer< CounterType, D > >
{
        using counter_traits = traits_for< CounterType >;
        using sub_traits     = traits_for< D >;
        using value_type     = typename sub_traits::value_type;

        static constexpr std::size_t max_size = counter_traits::max_size + sub_traits::max_size;
        static constexpr std::size_t min_size = counter_traits::min_size + sub_traits::min_size;
};

template < auto V >
struct proto_traits< tag< V > >
{
        using sub_traits = traits_for< decltype( V ) >;
        using value_type = tag< V >;

        static constexpr std::size_t max_size = sub_traits::max_size;
        static constexpr std::size_t min_size = sub_traits::min_size;
};

template < convertible... Ds >
struct proto_traits< group< Ds... > >
{
        using value_type = std::variant< typename traits_for< Ds >::value_type... >;

        static constexpr std::size_t max_size =
            std::max< std::size_t >( { traits_for< Ds >::max_size..., 0 } );
        static constexpr std::size_t min_size = std::min< std::size_t >(
            { traits_for< Ds >::min_size...,
              sizeof...( Ds ) == 0 ? 0 : std::numeric_limits< std::size_t >::max() } );
};

template < convertible... Ds >
struct proto_traits< tag_group< Ds... > >
{
        using value_type = std::variant< typename traits_for< Ds >::value_type... >;

        template < typename D >
        using to_tuple = std::tuple< tag< D::id >, D >;

        using sub_type   = group< to_tuple< Ds >... >;
        using sub_traits = traits_for< sub_type >;

        static constexpr std::size_t max_size = sub_traits::max_size;
        static constexpr std::size_t min_size = sub_traits::min_size;
};

template < std::endian Endianess, convertible D >
struct proto_traits< endianess_wrapper< Endianess, D > > : traits_for< D >
{
};

template < std::derived_from< converter_def_type_base > D >
struct proto_traits< D > : traits_for< typename D::def_type >
{
};

template < std::size_t N >
struct proto_traits< string_buffer< N > >
{
        using counter_type                    = uint16_t;
        using counter_traits                  = traits_for< counter_type >;
        using value_type                      = string_buffer< N >;
        static constexpr std::size_t max_size = N + counter_traits::max_size;
        static constexpr std::size_t min_size = 0 + counter_traits::min_size;
};

template <>
struct proto_traits< error_record >
{
        using value_type = error_record;

        using mark_type   = mark;
        using offset_type = std::size_t;

        using mark_traits   = traits_for< mark_type >;
        using offset_traits = traits_for< offset_type >;

        static constexpr std::size_t max_size = mark_traits::max_size + offset_traits::max_size;
        static constexpr std::size_t min_size = mark_traits::min_size + offset_traits::min_size;
};

template < convertible T, std::size_t N >
struct proto_traits< static_vector< T, N > >
{
        using value_type   = static_vector< T, N >;
        using counter_type = uint16_t;
        static constexpr std::size_t max_size =
            traits_for< counter_type >::max_size + traits_for< T >::max_size * N;
        static constexpr std::size_t min_size = traits_for< counter_type >::min_size;
};

template < convertible T >
struct proto_traits< std::optional< T > >
{
        using value_type      = std::optional< T >;
        using presence_type   = bounded< uint8_t, 0, 1 >;
        using presence_traits = traits_for< presence_type >;
        static constexpr std::size_t max_size =
            presence_traits::max_size + traits_for< T >::max_size;
        static constexpr std::size_t min_size = presence_traits::min_size;
};

template < decomposable T >
struct backup_proto_traits< T >
{
        using value_type                      = T;
        using tuple_type                      = decomposed_type< T >;
        using tuple_traits                    = traits_for< tuple_type >;
        static constexpr std::size_t max_size = tuple_traits::max_size;
        static constexpr std::size_t min_size = tuple_traits::min_size;
};

}  // namespace emlabcpp::protocol
