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
#include "emlabcpp/algorithm.h"
#include "emlabcpp/assert.h"
#include "emlabcpp/either.h"
#include "emlabcpp/iterators/numeric.h"
#include "emlabcpp/match.h"
#include "emlabcpp/protocol/serializer.h"
#include "emlabcpp/protocol/traits.h"

#pragma once

namespace emlabcpp::protocol
{

/// converter<T,E> structure defines how type T should be serialized and deserialized. Each type
/// or kind of types should overlead this structure and use same attributes as traits_for<T>. E
/// is edianess of the serialization used.
template < typename, std::endian >
struct converter;

template < typename, std::endian >
struct backup_converter;

template < typename D, std::endian E >
auto converter_for_impl()
{
        if constexpr ( with_value_type< proto_traits< D > > ) {
                return converter< D, E >{};
        } else {
                return backup_converter< D, E >{};
        }
}

template < typename D, std::endian E >
using converter_for = decltype( converter_for_impl< D, E >() );

/// converter_check<T> concept verifies that 'T' is valid overload of converter. Use this in
/// tests of custom converter overloads.
template < typename T >
concept converter_check = requires()
{
        {
                T::max_size
                } -> std::convertible_to< std::size_t >;
        typename T::value_type;
        requires bounded_derived< typename T::size_type >;
}
&&requires( std::span< uint8_t, T::max_size > buff, typename T::value_type item )
{
        {
                T::serialize_at( buff, item )
                } -> std::same_as< typename T::size_type >;
}
&&requires( std::span< const uint8_t > buff, typename T::value_type item )
{
        T::deserialize( buff, item );
};

template < base_type D, std::endian Endianess >
struct converter< D, Endianess >
{
        using value_type                      = typename traits_for< D >::value_type;
        static constexpr std::size_t max_size = traits_for< D >::max_size;
        using size_type                       = bounded< std::size_t, max_size, max_size >;

        static constexpr bool is_big_endian = Endianess == std::endian::big;

        static constexpr auto& bget( auto& buffer, const std::size_t i )
        {
                return buffer[is_big_endian ? i : max_size - 1 - i];
        }

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, value_type item )
        {
                serializer< value_type, Endianess >::serialize_at( buffer, item );
                return size_type{};
        }

        static constexpr conversion_result
        deserialize( const std::span< const uint8_t >& buffer, value_type& value )
        {
                if ( buffer.size() < max_size ) {
                        return { 0, &SIZE_ERR };
                }

                value =
                    serializer< value_type, Endianess >::deserialize( buffer.first< max_size >() );

                return conversion_result{ max_size };
        }
};

template < convertible D, std::size_t N, std::endian Endianess >
struct converter< std::array< D, N >, Endianess >
{
        using value_type = typename traits_for< std::array< D, N > >::value_type;
        static constexpr std::size_t max_size = traits_for< std::array< D, N > >::max_size;
        static constexpr std::size_t min_size = traits_for< std::array< D, N > >::min_size;

        using sub_converter = converter_for< D, Endianess >;
        using sub_size_type = typename sub_converter::size_type;
        using size_type     = bounded< std::size_t, min_size, max_size >;

        /// In both methods, we create the bounded size without properly checking that it the
        /// bounded type was made properly (that is, that the provided std::size_t value is in the
        /// range). Thas is ok as long as variant "we advanced the iter only by at max
        /// `sub_converter::max_size` `N` times".

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                auto iter = buffer.begin();

                for ( const std::size_t i : range( N ) ) {
                        std::span< uint8_t, sub_converter::max_size > sub_view{
                            iter, sub_converter::max_size };

                        bounded bused = sub_converter::serialize_at( sub_view, item[i] );

                        std::advance( iter, *bused );
                }

                auto used      = std::distance( buffer.begin(), iter );
                auto opt_bused = size_type::make( used );
                EMLABCPP_ASSERT( opt_bused );
                return *opt_bused;
        }

        static constexpr conversion_result
        deserialize( const std::span< const uint8_t >& buffer, value_type& value )
        {

                std::size_t offset = 0;

                for ( const std::size_t i : range( N ) ) {
                        if ( offset > buffer.size() ) {
                                return { offset, &SIZE_ERR };
                        }
                        std::span subspan = buffer.subspan( offset );

                        auto sres = sub_converter::deserialize( subspan, value[i] );
                        if ( sres.has_error() ) {
                                return sres;
                        }
                        offset += sres.used;
                }

                return conversion_result{ offset };
        }
};

template < convertible... Ds, std::endian Endianess >
struct converter< std::tuple< Ds... >, Endianess >
{
        using def_type = std::tuple< Ds... >;

        using value_type                      = typename traits_for< def_type >::value_type;
        static constexpr std::size_t max_size = traits_for< def_type >::max_size;
        static constexpr std::size_t min_size = traits_for< def_type >::min_size;
        using size_type                       = bounded< std::size_t, min_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                auto iter = buffer.begin();

                for_each_index< sizeof...( Ds ) >( [&iter, &item]< std::size_t i >() {
                        using sub_converter =
                            converter_for< std::tuple_element_t< i, def_type >, Endianess >;

                        std::span< uint8_t, sub_converter::max_size > sub_view{
                            iter, sub_converter::max_size };

                        bounded bused =
                            sub_converter::serialize_at( sub_view, std::get< i >( item ) );

                        std::advance( iter, *bused );
                } );

                auto used      = std::distance( buffer.begin(), iter );
                auto opt_bused = size_type::make( used );
                EMLABCPP_ASSERT( opt_bused );
                return *opt_bused;
        }

        template < std::size_t I >
        using nth_converter = converter_for< std::tuple_element_t< I, def_type >, Endianess >;

        static constexpr conversion_result
        deserialize( const std::span< const uint8_t >& buffer, value_type& value )
        {

                std::size_t offset = 0;
                const mark* err    = nullptr;

                until_index< sizeof...( Ds ) >(
                    [&offset, &err, &value, &buffer]< std::size_t i >() {
                            if ( offset > buffer.size() ) {
                                    err = &SIZE_ERR;
                                    return true;
                            }
                            std::span subspan = buffer.subspan( offset );

                            auto sres =
                                nth_converter< i >::deserialize( subspan, std::get< i >( value ) );

                            offset += sres.used;

                            if ( sres.has_error() ) {
                                    err = sres.get_error();
                                    return true;
                            }
                            return false;
                    } );

                return conversion_result{ offset, err };
        }
};

// TODO: make a tests for variant with duplicated types
template < convertible... Ds, std::endian Endianess >
struct converter< std::variant< Ds... >, Endianess >
{
        using def_type                        = std::variant< Ds... >;
        using value_type                      = typename traits_for< def_type >::value_type;
        static constexpr std::size_t max_size = traits_for< def_type >::max_size;
        static constexpr std::size_t min_size = traits_for< def_type >::min_size;

        using id_type                        = uint8_t;
        using id_converter                   = converter_for< id_type, Endianess >;
        static constexpr std::size_t id_size = id_converter::max_size;

        static_assert(
            sizeof...( Ds ) < std::numeric_limits< id_type >::max(),
            "Number of items for variant is limited by the size of one byte - 256 items" );
        static_assert( fixedly_sized< id_type > );
        using size_type = bounded< std::size_t, min_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                id_converter::serialize_at(
                    buffer.template first< id_size >(), static_cast< id_type >( item.index() ) );

                return visit_index(
                    [&buffer, &item]< std::size_t i >() -> size_type {
                            using sub_converter = converter_for<
                                std::variant_alternative_t< i, def_type >,
                                Endianess >;

                            /// this also asserts that id has static serialized size
                            return bounded_constant< id_converter::max_size > +
                                   sub_converter::serialize_at(
                                       buffer.template subspan<
                                           id_converter::max_size,
                                           sub_converter::max_size >(),
                                       std::get< i >( item ) );
                    },
                    item );
        }

        template < std::size_t I >
        using nth_converter = converter_for< std::variant_alternative_t< I, def_type >, Endianess >;

        static constexpr conversion_result
        deserialize( const std::span< const uint8_t >& buffer, value_type& value )
        {
                id_type id;
                auto    subres = id_converter::deserialize( buffer, id );
                if ( subres.has_error() ) {
                        return subres;
                }

                auto subspan = buffer.subspan< id_size >();

                conversion_result res{ 0, &UNDEFVAR_ERR };

                until_index< sizeof...( Ds ) >(
                    [&res, &subres, &subspan, &id, &value]< std::size_t i >() {
                            if ( id != i ) {
                                    return false;
                            }

                            res = nth_converter< i >::deserialize(
                                subspan, value.template emplace< i >() );
                            res.used += subres.used;
                            return true;
                    } );

                return res;
        }
};

template < std::endian Endianess >
struct converter< std::monostate, Endianess >
{
        using value_type                      = std::monostate;
        static constexpr std::size_t max_size = 0;
        using size_type                       = bounded< std::size_t, 0, 0 >;

        static constexpr size_type serialize_at( const std::span< uint8_t, 0 >, const value_type& )
        {
                return size_type{};
        }

        static constexpr conversion_result
        deserialize( const std::span< const uint8_t >&, value_type& )
        {
                return conversion_result{ 0 };
        }
};

template < convertible T, std::endian Endianess >
struct converter< std::optional< T >, Endianess >
{
        using traits                          = traits_for< std::optional< T > >;
        using value_type                      = std::optional< T >;
        static constexpr std::size_t max_size = traits::max_size;
        static constexpr std::size_t min_size = traits::min_size;
        using presence_type                   = typename traits::presence_type;

        static_assert( fixedly_sized< presence_type > );

        using presence_converter                   = converter_for< presence_type, Endianess >;
        static constexpr std::size_t presence_size = presence_converter::max_size;
        using sub_converter                        = converter_for< T, Endianess >;
        using sub_size                             = typename sub_converter::size_type;
        using size_type                            = bounded< std::size_t, min_size, max_size >;

        static constexpr presence_type is_present  = bounded< uint8_t, 0, 1 >::get< 1 >();
        static constexpr presence_type not_present = bounded< uint8_t, 0, 1 >::get< 0 >();

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& opt_val )
        {
                if ( !opt_val ) {
                        return presence_converter::serialize_at(
                            buffer.template first< presence_size >(), not_present );
                }
                auto psize = presence_converter::serialize_at(
                    buffer.template first< presence_size >(), is_present );

                return psize +
                       sub_converter::serialize_at(
                           buffer.template subspan< presence_size, sub_converter::max_size >(),
                           *opt_val );
        }

        static constexpr conversion_result
        deserialize( const std::span< const uint8_t >& buffer, value_type& value )
        {
                presence_type is_present_v;
                auto          subres = presence_converter::deserialize(
                    buffer.first< presence_size >(), is_present_v );
                if ( subres.has_error() ) {
                        return subres;
                }

                if ( is_present_v == not_present ) {
                        return subres;
                }

                std::span subspan = buffer.subspan( presence_size );

                auto res = sub_converter::deserialize( subspan, value.emplace() );
                res.used += subres.used;
                return res;
        }
};

template < std::size_t N, std::endian Endianess >
struct converter< std::bitset< N >, Endianess >
{
        using value_type                      = typename traits_for< std::bitset< N > >::value_type;
        static constexpr std::size_t max_size = traits_for< std::bitset< N > >::max_size;
        using size_type                       = bounded< std::size_t, max_size, max_size >;

        static constexpr bool is_big_endian = Endianess == std::endian::big;

        static constexpr auto& bget( auto& buffer, const std::size_t i )
        {
                return buffer[is_big_endian ? i : max_size - 1 - i];
        }

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                for ( const std::size_t i : range( max_size ) ) {
                        std::bitset< 8 > byte;
                        for ( const std::size_t j : range( 8u ) ) {
                                byte[j] = item[i * 8 + j];
                        }
                        bget( buffer, i ) = static_cast< uint8_t >( byte.to_ulong() );
                }
                return size_type{};
        }

        static constexpr conversion_result
        deserialize( const std::span< const uint8_t >& buffer, value_type& value )
        {
                if ( buffer.size() < max_size ) {
                        return { 0, &SIZE_ERR };
                }
                for ( const std::size_t i : range( max_size ) ) {
                        std::bitset< 8 > byte = bget( buffer, i );
                        for ( const std::size_t j : range( 8u ) ) {
                                value[i * 8 + j] = byte[j];
                        }
                }
                return conversion_result{ max_size };
        }
};

template < std::size_t N, std::endian Endianess >
struct converter< message< N >, Endianess >
{
        using traits_type                     = traits_for< message< N > >;
        using value_type                      = typename traits_type::value_type;
        static constexpr std::size_t min_size = traits_type::min_size;
        static constexpr std::size_t max_size = traits_type::max_size;
        using size_type                       = bounded< std::size_t, min_size, max_size >;
        using msg_size_type                   = typename traits_type::msg_size_type;
        static_assert( fixedly_sized< msg_size_type > );
        using msg_size_converter                   = converter_for< msg_size_type, Endianess >;
        static constexpr std::size_t msg_size_size = msg_size_converter::max_size;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                auto used = msg_size_converter::serialize_at(
                    buffer.template first< msg_size_size >(),
                    static_cast< msg_size_type >( item.size() ) );

                for ( const std::size_t i : range( item.size() ) ) {
                        buffer[i + msg_size_size] = item[i];
                }
                /// The size of protocol::message should always be within the 0...N range
                auto opt_bused = size_type::make( item.size() + *used );
                EMLABCPP_ASSERT( opt_bused );
                return *opt_bused;
        }

        static constexpr conversion_result
        deserialize( const std::span< const uint8_t >& buffer, value_type& value )
        {
                msg_size_type size;
                auto          subres = msg_size_converter::deserialize( buffer, size );
                if ( subres.has_error() ) {
                        return subres;
                }
                if ( buffer.size() < subres.used + size ) {
                        return { subres.used, &SIZE_ERR };
                }
                auto opt_msg = value_type::make( buffer.subspan( subres.used, size ) );

                if ( !opt_msg ) {
                        return { msg_size_size, &BIGSIZE_ERR };
                }
                value = *opt_msg;
                return conversion_result{ opt_msg->size() + subres.used };
        }
};

template < std::size_t N, std::endian Endianess >
struct converter< sizeless_message< N >, Endianess >
{
        using value_type = typename traits_for< sizeless_message< N > >::value_type;
        static constexpr std::size_t max_size = traits_for< sizeless_message< N > >::max_size;
        using size_type                       = bounded< std::size_t, 0, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                for ( const std::size_t i : range( item.size() ) ) {
                        buffer[i] = item[i];
                }
                /// The size of protocol::message should always be within the 0...N range
                auto opt_bused = size_type::make( item.size() );
                EMLABCPP_ASSERT( opt_bused );
                return *opt_bused;
        }

        static constexpr conversion_result
        deserialize( const std::span< const uint8_t >& buffer, value_type& value )
        {
                auto opt_msg = value_type::make( buffer );

                if ( !opt_msg ) {
                        return { 0, &BIGSIZE_ERR };
                }
                value = *opt_msg;
                return conversion_result{ opt_msg->size() };
        }
};

template < convertible D, auto Offset, std::endian Endianess >
struct converter< value_offset< D, Offset >, Endianess >
{
        using value_type = typename traits_for< value_offset< D, Offset > >::value_type;
        static constexpr std::size_t max_size = traits_for< value_offset< D, Offset > >::max_size;

        using sub_converter = converter_for< D, Endianess >;
        using size_type     = typename sub_converter::size_type;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                return sub_converter::serialize_at( buffer, item + Offset );
        }

        static constexpr conversion_result
        deserialize( const std::span< const uint8_t >& buffer, value_type& value )
        {
                auto subres = sub_converter::deserialize( buffer, value );

                value -= Offset;

                return subres;
        }
};

template < quantity_derived D, std::endian Endianess >
struct converter< D, Endianess >
{
        using value_type                      = typename traits_for< D >::value_type;
        static constexpr std::size_t max_size = traits_for< D >::max_size;

        using inner_type = typename D::value_type;

        static_assert( convertible< inner_type > );

        using sub_converter = converter_for< inner_type, Endianess >;
        using size_type     = typename sub_converter::size_type;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                return sub_converter::serialize_at( buffer, *item );
        }

        static constexpr conversion_result
        deserialize( const std::span< const uint8_t >& buffer, value_type& value )
        {
                inner_type val;
                auto       sub_res = sub_converter::deserialize( buffer, val );
                value              = D{ val };
                return sub_res;
        }
};

template < convertible D, D Min, D Max, std::endian Endianess >
struct converter< bounded< D, Min, Max >, Endianess >
{
        using value_type = typename traits_for< bounded< D, Min, Max > >::value_type;
        static constexpr std::size_t max_size = traits_for< bounded< D, Min, Max > >::max_size;

        using sub_converter = converter_for< D, Endianess >;
        using size_type     = typename sub_converter::size_type;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                return sub_converter::serialize_at( buffer, *item );
        }

        static constexpr conversion_result
        deserialize( const std::span< const uint8_t >& buffer, value_type& value )
        {
                D    sub_val;
                auto subres = sub_converter::deserialize( buffer, sub_val );
                if ( subres.has_error() ) {
                        return subres;
                }

                auto opt_val = value_type::make( sub_val );
                if ( !opt_val ) {
                        return { 0, &BOUNDS_ERR };
                }
                value = *opt_val;
                return subres;
        }
};

template < convertible CounterDef, convertible D, std::endian Endianess >
struct converter< sized_buffer< CounterDef, D >, Endianess >
{
        using value_type = typename traits_for< sized_buffer< CounterDef, D > >::value_type;
        static constexpr std::size_t max_size =
            traits_for< sized_buffer< CounterDef, D > >::max_size;
        static constexpr std::size_t min_size =
            traits_for< sized_buffer< CounterDef, D > >::min_size;

        using sub_converter = converter_for< D, Endianess >;

        using counter_converter                   = converter_for< CounterDef, Endianess >;
        using counter_size_type                   = typename counter_converter::size_type;
        using counter_type                        = typename counter_converter::value_type;
        static constexpr std::size_t counter_size = counter_converter::max_size;

        /// we expect that counter item does not have dynamic size
        static_assert( fixedly_sized< CounterDef > );

        using size_type = bounded< std::size_t, min_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                auto vused = sub_converter::serialize_at(
                    buffer.template last< sub_converter::max_size >(), item );
                auto cused = counter_converter::serialize_at(
                    buffer.template first< counter_size >(),
                    static_cast< counter_type >( *vused ) );
                return vused + cused;
        }

        static constexpr conversion_result
        deserialize( const std::span< const uint8_t >& buffer, value_type& value )
        {
                counter_type size = 0;
                auto         cres = counter_converter::deserialize( buffer, size );
                if ( cres.has_error() ) {
                        return cres;
                }
                if ( buffer.size() < cres.used + size ) {
                        return { cres.used, &SIZE_ERR };
                }
                auto subres =
                    sub_converter::deserialize( buffer.subspan( counter_size, size ), value );
                subres.used += cres.used;
                return subres;
        }
};

template < auto V, std::endian Endianess >
struct converter< tag< V >, Endianess >
{
        using value_type                      = typename traits_for< tag< V > >::value_type;
        static constexpr std::size_t max_size = traits_for< tag< V > >::max_size;

        using sub_converter = converter_for< decltype( V ), Endianess >;

        using size_type = typename sub_converter::size_type;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& )
        {
                return sub_converter::serialize_at( buffer, V );
        }

        static constexpr conversion_result
        deserialize( const std::span< const uint8_t >& buffer, value_type& )
        {
                decltype( V ) val;
                auto          subres = sub_converter::deserialize( buffer, val );
                if ( subres.has_error() ) {
                        return subres;
                }
                if ( val != V ) {
                        return { 0, &BADVAL_ERR };
                }
                return conversion_result{ subres.used };
        }
};

template < typename... Ds, std::endian Endianess >
struct converter< tag_group< Ds... >, Endianess >
{
        using traits                          = traits_for< tag_group< Ds... > >;
        using value_type                      = typename traits::value_type;
        static constexpr std::size_t max_size = traits::max_size;
        static constexpr std::size_t min_size = traits::min_size;

        using def_variant = std::variant< Ds... >;
        using size_type   = bounded< std::size_t, min_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                return visit_index(
                    [&buffer, &item]< std::size_t i >() -> size_type {
                            auto tag_used = nth_tag_converter< i >::serialize_at(
                                buffer.template subspan< 0, nth_tag_converter< i >::max_size >(),
                                nth_tag< i >{} );
                            auto& val  = std::get< i >( item );
                            auto  used = nth_converter< i >::serialize_at(
                                buffer.template subspan<
                                    nth_tag_converter< i >::max_size,
                                    nth_converter< i >::max_size >(),
                                val );
                            return tag_used + used;
                    },
                    item );
        }

        template < std::size_t I >
        using nth_converter =
            converter_for< std::variant_alternative_t< I, def_variant >, Endianess >;

        template < std::size_t I >
        using nth_tag = tag< std::variant_alternative_t< I, def_variant >::id >;

        template < std::size_t I >
        using nth_tag_converter = converter_for< nth_tag< I >, Endianess >;

        static constexpr conversion_result
        deserialize( const std::span< const uint8_t >& buffer, value_type& value )
        {
                conversion_result res;
                until_index< sizeof...( Ds ) >( [&buffer, &value, &res]< std::size_t i >() -> bool {
                        nth_tag< i > tag;
                        auto         tag_res = nth_tag_converter< i >::deserialize( buffer, tag );
                        if ( tag_res.has_error() ) {
                                return false;
                        }

                        res = nth_converter< i >::deserialize(
                            buffer.subspan( tag_res.used ), value.template emplace< i >() );
                        res.used += tag_res.used;
                        return true;
                } );
                return res;
        }
};

template < typename... Ds, std::endian Endianess >
struct converter< group< Ds... >, Endianess >
{
        using value_type                      = typename traits_for< group< Ds... > >::value_type;
        static constexpr std::size_t max_size = traits_for< group< Ds... > >::max_size;
        static constexpr std::size_t min_size = traits_for< group< Ds... > >::min_size;

        using def_variant = std::variant< Ds... >;
        using size_type   = bounded< std::size_t, min_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                return visit_index(
                    [&buffer, &item]< std::size_t i >() -> size_type {
                            using sub_converter = converter_for<
                                std::variant_alternative_t< i, def_variant >,
                                Endianess >;
                            return sub_converter::serialize_at(
                                buffer.template subspan< 0, sub_converter::max_size >(),
                                std::get< i >( item ) );
                    },
                    item );
        }

        template < std::size_t I >
        using nth_converter =
            converter_for< std::variant_alternative_t< I, def_variant >, Endianess >;

        static constexpr conversion_result
        deserialize( const std::span< const uint8_t >& buffer, value_type& value )
        {
                conversion_result res;

                const bool got_match =
                    until_index< sizeof...( Ds ) >( [&buffer, &res, &value]< std::size_t i >() {
                            res = nth_converter< i >::deserialize(
                                buffer, value.template emplace< i >() );

                            if ( res.used == 0 ) {
                                    return false;
                            }
                            return true;
                    } );

                if ( got_match ) {
                        return res;
                }

                return { 0, &GROUP_ERR };
        }
};

template < std::endian Endianess, typename D, std::endian ParentEndianess >
struct converter< endianess_wrapper< Endianess, D >, ParentEndianess >
  : converter_for< D, Endianess >
{
};

template < std::derived_from< converter_def_type_base > D, std::endian Endianess >
struct converter< D, Endianess > : converter_for< typename D::def_type, Endianess >
{
};

template < std::endian Endianess >
struct converter< mark, Endianess >
{
        using value_type                      = typename traits_for< mark >::value_type;
        static constexpr std::size_t max_size = traits_for< mark >::max_size;
        using size_type                       = bounded< std::size_t, max_size, max_size >;

        static constexpr size_type
        serialize_at( const std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                copy( item, buffer.begin() );
                return size_type{};
        }

        static constexpr conversion_result
        deserialize( const std::span< const uint8_t >& buffer, value_type& value )
        {
                if ( buffer.size() < max_size ) {
                        return { 0, &SIZE_ERR };
                }
                copy( view_n( buffer.begin(), max_size ), value.begin() );
                return conversion_result{ max_size };
        }
};

template < std::endian Endianess >
struct converter< error_record, Endianess >
{
        using traits                          = traits_for< error_record >;
        using value_type                      = typename traits::value_type;
        using mark_converter                  = converter_for< mark, Endianess >;
        using offset_converter                = converter_for< std::size_t, Endianess >;
        static constexpr std::size_t max_size = traits::max_size;
        using size_type                       = bounded< std::size_t, max_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, value_type item )
        {
                mark_converter::serialize_at(
                    buffer.first< mark_converter::max_size >(), item.error_mark );
                offset_converter::serialize_at(
                    buffer.last< offset_converter::max_size >(), item.offset );
                return size_type{};
        }

        static constexpr conversion_result
        deserialize( const std::span< const uint8_t >& buffer, value_type& value )
        {

                auto subres = mark_converter::deserialize( buffer, value.error_mark );
                if ( subres.has_error() ) {
                        return subres;
                }
                return offset_converter::deserialize(
                    buffer.subspan< mark_converter::max_size >(), value.offset );
        }
};

template < typename T, std::size_t N, std::endian Endianess >
struct converter< static_vector< T, N >, Endianess >
{
        using value_type = typename traits_for< static_vector< T, N > >::value_type;

        static_assert( N <= std::numeric_limits< uint16_t >::max() );

        using counter_type      = typename traits_for< static_vector< T, N > >::counter_type;
        using counter_converter = converter_for< counter_type, Endianess >;
        using sub_converter     = converter_for< T, Endianess >;

        static constexpr std::size_t counter_size = counter_converter::max_size;

        static_assert( fixedly_sized< counter_type > );

        static constexpr std::size_t max_size = traits_for< static_vector< T, N > >::max_size;
        static constexpr std::size_t min_size = traits_for< static_vector< T, N > >::min_size;
        using size_type                       = bounded< std::size_t, min_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, value_type item )
        {

                counter_converter::serialize_at(
                    buffer.template first< counter_size >(),
                    static_cast< counter_type >( item.size() ) );

                /// TODO: this duplicates std::array, generalize?
                auto iter = buffer.begin() + counter_size;
                for ( const std::size_t i : range( item.size() ) ) {
                        std::span< uint8_t, sub_converter::max_size > sub_view{
                            iter, sub_converter::max_size };

                        bounded sub_bused = sub_converter::serialize_at( sub_view, item[i] );

                        std::advance( iter, *sub_bused );
                }

                auto used = static_cast< std::size_t >( std::distance( buffer.begin(), iter ) );
                auto opt_bused = size_type::make( used );
                EMLABCPP_ASSERT( opt_bused );

                return *opt_bused;
        }

        static constexpr conversion_result
        deserialize( const std::span< const uint8_t >& buffer, value_type& value )
        {
                counter_type count;
                auto         subres = counter_converter::deserialize( buffer, count );
                if ( subres.has_error() ) {
                        return subres;
                }
                std::size_t offset = subres.used;

                for ( const std::size_t i : range( count ) ) {
                        std::ignore = i;
                        if ( offset > buffer.size() ) {
                                return { offset, &SIZE_ERR };
                        }
                        std::span subspan = buffer.subspan( offset );
                        value.emplace_back();

                        auto itemres = sub_converter::deserialize( subspan, value.back() );
                        offset += itemres.used;
                        if ( itemres.has_error() ) {
                                return { offset, itemres.get_error() };
                        }
                }

                return conversion_result{ offset };
        }
};

template < decomposable T, std::endian Endianess >
struct backup_converter< T, Endianess >
{
        using traits                          = traits_for< T >;
        using value_type                      = typename traits::value_type;
        static constexpr std::size_t max_size = traits::max_size;
        static constexpr std::size_t min_size = traits::min_size;

        using size_type = bounded< std::size_t, min_size, max_size >;

        using tuple_type    = typename traits::tuple_type;
        using sub_converter = converter_for< tuple_type, Endianess >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                tuple_type tpl = decompose( item );
                return sub_converter::serialize_at( buffer, tpl );
        }

        static constexpr conversion_result
        deserialize( const std::span< const uint8_t >& buffer, value_type& value )
        {
                tuple_type val;
                auto       subres = sub_converter::deserialize( buffer, val );
                if ( subres.has_error() ) {
                        return subres;
                }
                value = compose< T >( val );
                return subres;
        }
};

template < typename T, std::endian Endianess >
struct memcpy_converter
{
        using traits                          = traits_for< T >;
        using value_type                      = typename traits::value_type;
        static constexpr std::size_t min_size = traits::min_size;
        static constexpr std::size_t max_size = traits::max_size;
        using size_type                       = bounded< std::size_t, min_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, value_type item )
        {
                std::memcpy( buffer.begin(), &item, sizeof( value_type ) );
        }

        static constexpr conversion_result
        deserialize( const std::span< const uint8_t >& buffer, value_type& value )
        {
                if ( buffer.size() < max_size ) {
                        return { 0, &SIZE_ERR };
                }

                std::memcpy( &value, buffer.begin(), sizeof( value_type ) );
                return conversion_result{ max_size };
        }
};

}  // namespace emlabcpp::protocol
