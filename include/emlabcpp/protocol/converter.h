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
#include "emlabcpp/experimental/bounded_view.h"
#include "emlabcpp/iterators/numeric.h"
#include "emlabcpp/match.h"
#include "emlabcpp/protocol/traits.h"
#include "emlabcpp/protocol/serializer.h"

#pragma once

namespace emlabcpp::protocol
{

/// converter<T,E> structure defines how type T should be serialized and deserialized. Each type
/// or kind of types should overlead this structure and use same attributes as proto_traits<T,E>. E
/// is edianess of the serialization used.
template < typename, endianess_enum >
struct converter;

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
&&requires( bounded_view< const uint8_t*, typename T::size_type > buff )
{
        {
                T::deserialize( buff )
                } -> std::same_as< conversion_result< typename T::value_type > >;
};

template < base_type D, endianess_enum Endianess >
struct converter< D, Endianess >
{
        using value_type                      = typename proto_traits< D >::value_type;
        static constexpr std::size_t max_size = proto_traits< D >::max_size;
        using size_type                       = bounded< std::size_t, max_size, max_size >;

        static constexpr bool is_big_endian = Endianess == PROTOCOL_BIG_ENDIAN;

        static constexpr auto& bget( auto& buffer, std::size_t i )
        {
                return buffer[is_big_endian ? i : max_size - 1 - i];
        }

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, value_type item )
        {
                serializer< value_type, Endianess >::serialize_at( buffer, item );
                return size_type{};
        }

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> conversion_result< value_type >
        {
                return {
                    max_size, serializer< value_type, Endianess >::deserialize( buffer ) };
        }
};

template < convertible D, std::size_t N, endianess_enum Endianess >
struct converter< std::array< D, N >, Endianess >
{
        using value_type = typename proto_traits< std::array< D, N > >::value_type;
        static constexpr std::size_t max_size = proto_traits< std::array< D, N > >::max_size;
        static constexpr std::size_t min_size = proto_traits< std::array< D, N > >::min_size;

        using sub_def       = converter< D, Endianess >;
        using sub_size_type = typename sub_def::size_type;
        using size_type     = bounded< std::size_t, min_size, max_size >;

        /// In both methods, we create the bounded size without properly checking that it the
        /// bounded type was made properly (that is, that the provided std::size_t value is in the
        /// range). Thas is ok as long as variant "we advanced the iter only by at max
        /// `sub_def::max_size` `N` times".

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                auto iter = buffer.begin();

                for ( std::size_t i : range( N ) ) {
                        std::span< uint8_t, sub_def::max_size > sub_view{ iter, sub_def::max_size };

                        bounded bused = sub_def::serialize_at( sub_view, item[i] );

                        std::advance( iter, *bused );
                }

                auto used      = std::distance( buffer.begin(), iter );
                auto opt_bused = size_type::make( used );
                EMLABCPP_ASSERT( opt_bused );
                return *opt_bused;
        }

        static constexpr auto deserialize( bounded_view< const uint8_t*, size_type > buffer )
            -> conversion_result< value_type >
        {
                value_type  res{};
                std::size_t offset = 0;

                for ( std::size_t i : range( N ) ) {
                        auto opt_view = buffer.template opt_offset< sub_size_type >( offset );

                        if ( !opt_view ) {
                                return { offset, &SIZE_ERR };
                        }

                        auto [used, subres] = sub_def::deserialize( *opt_view );
                        offset += used;

                        if ( std::holds_alternative< const mark* >( subres ) ) {
                                return { offset, *std::get_if< const mark* >( &subres ) };
                        } else {
                                res[i] = *std::get_if< 0 >( &subres );
                        }
                }

                return conversion_result{ offset, res };
        }
};

template < convertible... Ds, endianess_enum Endianess >
struct converter< std::tuple< Ds... >, Endianess >
{
        using def_type = std::tuple< Ds... >;

        using value_type                      = typename proto_traits< def_type >::value_type;
        static constexpr std::size_t max_size = proto_traits< def_type >::max_size;
        static constexpr std::size_t min_size =
            ( converter< Ds, Endianess >::size_type::min_val + ... + 0 );
        using size_type = bounded< std::size_t, min_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                auto iter = buffer.begin();

                for_each_index< sizeof...( Ds ) >( [&]< std::size_t i >() {
                        using sub_def =
                            converter< std::tuple_element_t< i, def_type >, Endianess >;

                        std::span< uint8_t, sub_def::max_size > sub_view{ iter, sub_def::max_size };

                        bounded bused = sub_def::serialize_at( sub_view, std::get< i >( item ) );

                        std::advance( iter, *bused );
                } );

                auto used      = std::distance( buffer.begin(), iter );
                auto opt_bused = size_type::make( used );
                EMLABCPP_ASSERT( opt_bused );
                return *opt_bused;
        }

        static constexpr auto deserialize( bounded_view< const uint8_t*, size_type > buffer )
            -> conversion_result< value_type >
        {
                value_type res;

                std::size_t                           offset = 0;
                std::optional< const mark* > opt_err;

                until_index< sizeof...( Ds ) >( [&]< std::size_t i >() {
                        using D       = std::tuple_element_t< i, def_type >;
                        using sub_def = converter< D, Endianess >;

                        auto opt_view =
                            buffer.template opt_offset< typename sub_def::size_type >( offset );

                        if ( !opt_view ) {
                                opt_err = &SIZE_ERR;
                                return true;
                        }

                        auto [sused, sres] = sub_def::deserialize( *opt_view );
                        offset += sused;
                        if ( std::holds_alternative< const mark* >( sres ) ) {
                                opt_err = *std::get_if< const mark* >( &sres );
                                return true;
                        } else {
                                std::get< i >( res ) = *std::get_if< 0 >( &sres );
                        }
                        return false;
                } );

                if ( opt_err ) {
                        return { offset, *opt_err };
                }
                return { offset, res };
        }
};

template < convertible... Ds, endianess_enum Endianess >
struct converter< std::variant< Ds... >, Endianess >
{
        using def_type                        = std::variant< Ds... >;
        using value_type                      = typename proto_traits< def_type >::value_type;
        static constexpr std::size_t max_size = proto_traits< def_type >::max_size;

        using id_type                        = uint8_t;
        using id_def                         = converter< id_type, Endianess >;
        static constexpr std::size_t id_size = id_def::max_size;

        static_assert(
            sizeof...( Ds ) < std::numeric_limits< id_type >::max(),
            "Number of items for variant is limited by the size of one byte - 256 items" );
        static_assert( fixedly_sized< id_type > );

        static constexpr std::size_t min_size =
            id_def::size_type::min_val +
            std::min( { converter< Ds, Endianess >::size_type::min_val... } );
        using size_type = bounded< std::size_t, min_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                id_def::serialize_at(
                    buffer.template first< id_size >(), static_cast< id_type >( item.index() ) );

                std::optional< size_type > opt_res;
                until_index< sizeof...( Ds ) >( [&]< std::size_t i >() {
                        if ( i != item.index() ) {
                                return false;
                        }
                        using sub_def =
                            converter< std::variant_alternative_t< i, def_type >, Endianess >;

                        /// this also asserts that id has static serialized size
                        opt_res =
                            bounded_constant< id_def::max_size > +
                            sub_def::serialize_at(
                                buffer.template subspan< id_def::max_size, sub_def::max_size >(),
                                std::get< i >( item ) );

                        return true;
                } );
                /// The only case in which this can fire is that iteration 0...sizeof...(Ds) above
                /// did not mathced item.index(), which should not be possible
                EMLABCPP_ASSERT( opt_res );
                return *opt_res;
        }

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> conversion_result< value_type >
        {
                conversion_result< value_type > res{ 0, &UNDEFVAR_ERR };

                auto [iused, idres] = id_def::deserialize( buffer.template first< id_size >() );
                std::size_t used    = iused;

                if ( std::holds_alternative< const mark* >( idres ) ) {
                        res.res = *std::get_if< const mark* >( &idres );
                        return res;
                }
                id_type id = std::get< 0 >( idres );

                auto item_view = buffer.template offset< id_size >();

                until_index< sizeof...( Ds ) >( [&]< std::size_t i >() {
                        using D       = std::variant_alternative_t< i, def_type >;
                        using sub_def = converter< D, Endianess >;

                        if ( id != i ) {
                                return false;
                        }

                        auto opt_view =
                            item_view.template opt_offset< typename sub_def::size_type >( 0 );

                        if ( !opt_view ) {
                                res.res = &SIZE_ERR;
                                return true;
                        }

                        auto [sused, sres] = sub_def::deserialize( *opt_view );
                        res.used           = used + sused;
                        if ( std::holds_alternative< const mark* >( sres ) ) {
                                res.res = *std::get_if< const mark* >( &sres );
                        } else {
                                res.res = value_type{ *std::get_if< 0 >( &sres ) };
                        }
                        return true;
                } );

                return res;
        }
};

template < endianess_enum Endianess >
struct converter< std::monostate, Endianess >
{
        using value_type                      = std::monostate;
        static constexpr std::size_t max_size = 0;
        using size_type                       = bounded< std::size_t, 0, 0 >;

        static constexpr size_type serialize_at( std::span< uint8_t, 0 >, const value_type& )
        {
                return size_type{};
        }

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& )
            -> conversion_result< value_type >
        {
                return { 0, std::monostate{} };
        }
};

template < convertible T, endianess_enum Endianess >
struct converter< std::optional< T >, Endianess >
{
        using decl                            = proto_traits< std::optional< T > >;
        using value_type                      = std::optional< T >;
        static constexpr std::size_t max_size = decl::max_size;
        static constexpr std::size_t min_size = decl::min_size;
        using presence_type                   = typename decl::presence_type;

        static_assert( fixedly_sized< presence_type > );

        using presence_def                         = converter< presence_type, Endianess >;
        static constexpr std::size_t presence_size = presence_def::max_size;
        using sub_def                              = converter< T, Endianess >;
        using size_type                            = bounded< std::size_t, min_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& opt_val )
        {
                if ( !opt_val ) {
                        return presence_def::serialize_at(
                            buffer.template first< presence_size >(), 0 );
                }
                auto psize =
                    presence_def::serialize_at( buffer.template first< presence_size >(), 1 );

                return psize + sub_def::serialize_at(
                                   buffer.template subspan< presence_size, sub_def::max_size >(),
                                   *opt_val );
        }

        static constexpr auto deserialize( bounded_view< const uint8_t*, size_type > buffer )
            -> conversion_result< value_type >
        {
                conversion_result< value_type > res{ 0, &BADVAL_ERR };

                auto [pused, pres] =
                    presence_def::deserialize( buffer.template first< presence_size >() );
                res.used = pused;

                if ( std::holds_alternative< const mark* >( pres ) ) {
                        const mark* m = *std::get_if< const mark* >( &pres );
                        res.res                = m;
                        return res;
                }
                auto is_present = *std::get_if< presence_type >( &pres );
                if ( is_present == 0 ) {
                        res.res = std::nullopt;
                        return res;
                }
                if ( is_present != 1 ) {
                        return res;
                }

                using sub_size = typename sub_def::size_type;
                auto opt_view  = buffer.template opt_offset< sub_size >( presence_size );

                if ( !opt_view ) {
                        return res;
                }

                auto [sused, sres] = sub_def::deserialize( *opt_view );

                res.used += sused;
                match(
                    sres,
                    [&]( T val ) {
                            res.res = std::make_optional( std::move( val ) );
                    },
                    [&]( const mark* m ) {
                            res.res = m;
                    } );
                return res;
        }
};

template < std::size_t N, endianess_enum Endianess >
struct converter< std::bitset< N >, Endianess >
{
        using value_type = typename proto_traits< std::bitset< N > >::value_type;
        static constexpr std::size_t max_size = proto_traits< std::bitset< N > >::max_size;
        using size_type                       = bounded< std::size_t, max_size, max_size >;

        static constexpr bool is_big_endian = Endianess == PROTOCOL_BIG_ENDIAN;

        static constexpr auto& bget( auto& buffer, std::size_t i )
        {
                return buffer[is_big_endian ? i : max_size - 1 - i];
        }

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                for ( std::size_t i : range( max_size ) ) {
                        std::bitset< 8 > byte;
                        for ( std::size_t j : range( 8u ) ) {
                                byte[j] = item[i * 8 + j];
                        }
                        bget( buffer, i ) = static_cast< uint8_t >( byte.to_ulong() );
                }
                return size_type{};
        }

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> conversion_result< value_type >
        {
                std::bitset< N > res;
                for ( std::size_t i : range( max_size ) ) {
                        std::bitset< 8 > byte = bget( buffer, i );
                        for ( std::size_t j : range( 8u ) ) {
                                res[i * 8 + j] = byte[j];
                        }
                }
                return { max_size, res };
        }
};

template < std::size_t N, endianess_enum Endianess >
struct converter< sizeless_message< N >, Endianess >
{
        using value_type = typename proto_traits< sizeless_message< N > >::value_type;
        static constexpr std::size_t max_size =
            proto_traits< sizeless_message< N > >::max_size;
        using size_type = bounded< std::size_t, 0, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                for ( std::size_t i : range( item.size() ) ) {
                        buffer[i] = item[i];
                }
                /// The size of protocol::message should always be within the 0...N range
                auto opt_bused = size_type::make( item.size() );
                EMLABCPP_ASSERT( opt_bused );
                return *opt_bused;
        }

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> conversion_result< value_type >
        {
                auto opt_msg = value_type::make( buffer );

                if ( !opt_msg ) {
                        return { 0, &BIGSIZE_ERR };
                }
                return conversion_result{ opt_msg->size(), *opt_msg };
        }
};

template < convertible D, auto Offset, endianess_enum Endianess >
struct converter< value_offset< D, Offset >, Endianess >
{
        using value_type = typename proto_traits< value_offset< D, Offset > >::value_type;
        static constexpr std::size_t max_size =
            proto_traits< value_offset< D, Offset > >::max_size;

        using sub_def   = converter< D, Endianess >;
        using size_type = typename sub_def::size_type;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                return sub_def::serialize_at( buffer, item + Offset );
        }

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> conversion_result< value_type >
        {
                auto [used, res] = sub_def::deserialize( buffer );
                if ( std::holds_alternative< const mark* >( res ) ) {
                        return { used, *std::get_if< const mark* >( &res ) };
                }
                return { used, static_cast< value_type >( *std::get_if< 0 >( &res ) - Offset ) };
        }
};

template < quantity_derived D, endianess_enum Endianess >
struct converter< D, Endianess >
{
        using value_type                      = typename proto_traits< D >::value_type;
        static constexpr std::size_t max_size = proto_traits< D >::max_size;

        using inner_type = typename D::value_type;

        static_assert( convertible< inner_type > );

        using sub_def   = converter< inner_type, Endianess >;
        using size_type = typename sub_def::size_type;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                return sub_def::serialize_at( buffer, *item );
        }

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> conversion_result< value_type >
        {
                auto [used, res] = sub_def::deserialize( buffer );
                if ( std::holds_alternative< const mark* >( res ) ) {
                        return { used, *std::get_if< const mark* >( &res ) };
                }
                return { used, value_type{ *std::get_if< 0 >( &res ) } };
        }
};

template < convertible D, D Min, D Max, endianess_enum Endianess >
struct converter< bounded< D, Min, Max >, Endianess >
{
        using value_type = typename proto_traits< bounded< D, Min, Max > >::value_type;
        static constexpr std::size_t max_size = proto_traits< bounded< D, Min, Max > >::max_size;

        using sub_def   = converter< D, Endianess >;
        using size_type = typename sub_def::size_type;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                return sub_def::serialize_at( buffer, *item );
        }

        using result_either = conversion_result< value_type >;

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> conversion_result< value_type >
        {
                auto [used, res] = sub_def::deserialize( buffer );
                if ( std::holds_alternative< const mark* >( res ) ) {
                        return { used, *std::get_if< const mark* >( &res ) };
                }
                auto opt_val = value_type::make( *std::get_if< 0 >( &res ) );
                if ( !opt_val ) {
                        return { 0, &BOUNDS_ERR };
                }
                return { used, *opt_val };
        }
};

template < convertible CounterDef, convertible D, endianess_enum Endianess >
struct converter< sized_buffer< CounterDef, D >, Endianess >
{
        using value_type =
            typename proto_traits< sized_buffer< CounterDef, D > >::value_type;
        static constexpr std::size_t max_size =
            proto_traits< sized_buffer< CounterDef, D > >::max_size;

        using sub_def = converter< D, Endianess >;

        using counter_def                         = converter< CounterDef, Endianess >;
        using counter_size_type                   = typename counter_def::size_type;
        using counter_type                        = typename counter_def::value_type;
        static constexpr std::size_t counter_size = counter_def::max_size;

        /// we expect that counter item does not have dynamic size
        static_assert( fixedly_sized< CounterDef > );

        static constexpr std::size_t min_size = counter_size + sub_def::size_type::min_val;

        using size_type = bounded< std::size_t, min_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                auto vused =
                    sub_def::serialize_at( buffer.template last< sub_def::max_size >(), item );
                counter_def::serialize_at(
                    buffer.template first< counter_size >(),
                    static_cast< counter_type >( *vused ) );
                return vused + counter_size_type{};
        }

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> conversion_result< value_type >
        {

                auto [cused, cres] =
                    counter_def::deserialize( buffer.template first< counter_size >() );

                if ( std::holds_alternative< const mark* >( cres ) ) {
                        return { cused, *std::get_if< const mark* >( &cres ) };
                }
                std::size_t used       = *std::get_if< 0 >( &cres );
                auto        start_iter = buffer.begin() + counter_size;

                auto opt_view = bounded_view< const uint8_t*, typename sub_def::size_type >::make(
                    view_n( start_iter, used ) );
                if ( !opt_view ) {
                        return { cused, &SIZE_ERR };
                }

                auto [sused, res] = sub_def::deserialize( *opt_view );
                return { cused + sused, res };
        }
};

template < auto V, endianess_enum Endianess >
struct converter< tag< V >, Endianess >
{
        using value_type                      = typename proto_traits< tag< V > >::value_type;
        static constexpr std::size_t max_size = proto_traits< tag< V > >::max_size;

        using sub_def = converter< decltype( V ), Endianess >;

        using size_type = typename sub_def::size_type;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& )
        {
                return sub_def::serialize_at( buffer, V );
        }

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> conversion_result< value_type >
        {
                auto [used, res] = sub_def::deserialize( buffer );
                if ( std::holds_alternative< const mark* >( res ) ) {
                        return { 0, *std::get_if< const mark* >( &res ) };
                }
                auto val = *std::get_if< 0 >( &res );
                if ( val != V ) {
                        return { 0, &BADVAL_ERR };
                }
                return { used, tag< V >{} };
        }
};

template < typename... Ds, endianess_enum Endianess >
struct converter< tag_group< Ds... >, Endianess >
{
        using decl                            = proto_traits< tag_group< Ds... > >;
        using value_type                      = typename decl::value_type;
        static constexpr std::size_t max_size = decl::max_size;
        static constexpr std::size_t min_size = decl::min_size;

        using def_variant = std::variant< Ds... >;
        using size_type   = bounded< std::size_t, min_size, max_size >;
        using sub_type    = typename decl::sub_type;
        using sub_def     = converter< sub_type, Endianess >;

        using sub_value = typename sub_def::value_type;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                std::optional< size_type > opt_res;
                until_index< sizeof...( Ds ) >( [&]< std::size_t i >() {
                        if ( i != item.index() ) {
                                return false;
                        }
                        using D = std::variant_alternative_t< i, def_variant >;

                        opt_res = sub_def::serialize_at(
                            buffer.template subspan< 0, sub_def::max_size >(),
                            std::make_tuple( tag< D::tag >{}, std::get< i >( item ) ) );
                        return true;
                } );
                /// same check as for std::variant
                EMLABCPP_ASSERT( opt_res );
                return *opt_res;
        }

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> conversion_result< value_type >
        {
                auto [used, sres] = sub_def::deserialize( buffer );
                if ( std::holds_alternative< const mark* >( sres ) ) {
                        return { used, *std::get_if< const mark* >( &sres ) };
                }

                return {
                    used,
                    visit(
                        [&]< typename Tag, typename T >(
                            const std::tuple< Tag, T >& pack ) -> value_type {
                                return std::get< 1 >( pack );
                        },
                        *std::get_if< 0 >( &sres ) ) };
        }
};

template < typename... Ds, endianess_enum Endianess >
struct converter< group< Ds... >, Endianess >
{
        using value_type = typename proto_traits< group< Ds... > >::value_type;
        static constexpr std::size_t max_size = proto_traits< group< Ds... > >::max_size;
        static constexpr std::size_t min_size = proto_traits< group< Ds... > >::min_size;

        using def_variant = std::variant< Ds... >;
        using size_type   = bounded< std::size_t, min_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                std::optional< size_type > opt_res;
                until_index< sizeof...( Ds ) >( [&]< std::size_t i >() {
                        if ( i != item.index() ) {
                                return false;
                        }
                        using sub_def =
                            converter< std::variant_alternative_t< i, def_variant >, Endianess >;
                        opt_res = sub_def::serialize_at(
                            buffer.template subspan< 0, sub_def::max_size >(),
                            std::get< i >( item ) );
                        return true;
                } );
                /// same check as for std::variant
                EMLABCPP_ASSERT( opt_res );
                return *opt_res;
        }

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> conversion_result< value_type >
        {
                std::size_t                                    offset = 0;
                std::size_t                                    size   = buffer.size();
                std::optional< conversion_result< value_type > > opt_res;

                until_index< sizeof...( Ds ) >( [&]< std::size_t i >() {
                        using sub_def =
                            converter< std::variant_alternative_t< i, def_variant >, Endianess >;

                        /// TODO: this pattern repeats mutliple times here
                        auto opt_view =
                            bounded_view< const uint8_t*, typename sub_def::size_type >::make(
                                view_n(
                                    buffer.begin() + offset,
                                    std::min( sub_def::max_size, size - offset ) ) );

                        if ( !opt_view ) {
                                return false;
                        }

                        auto [used, sres] = sub_def::deserialize( *opt_view );
                        if ( used == 0 ) {
                                return false;
                        }

                        if ( std::holds_alternative< const mark* >( sres ) ) {
                                opt_res.emplace(
                                    used, *std::get_if< const mark* >( &sres ) );
                        } else {
                                opt_res.emplace( used, value_type{ *std::get_if< 0 >( &sres ) } );
                        }
                        return true;
                } );

                if ( opt_res ) {
                        return *opt_res;
                }

                return { 0, &GROUP_ERR };
        }
};

template < endianess_enum Endianess, typename D, endianess_enum ParentEndianess >
struct converter< endianess_wrapper< Endianess, D >, ParentEndianess >
  : converter< D, Endianess >
{
};

template < std::derived_from< converter_def_type_base > D, endianess_enum Endianess >
struct converter< D, Endianess > : converter< typename D::def_type, Endianess >
{
};

template < endianess_enum Endianess >
struct converter< mark, Endianess >
{
        using value_type                      = typename proto_traits< mark >::value_type;
        static constexpr std::size_t max_size = proto_traits< mark >::max_size;
        using size_type                       = bounded< std::size_t, max_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, value_type item )
        {
                std::copy( item.begin(), item.end(), buffer.begin() );
                return size_type{};
        }

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> conversion_result< value_type >
        {
                value_type res{};
                std::copy( buffer.begin(), buffer.begin() + max_size, res.begin() );
                return { 0, res };
        }
};

template < endianess_enum Endianess >
struct converter< error_record, Endianess >
{
        using decl                            = proto_traits< error_record >;
        using value_type                      = typename decl::value_type;
        using mark_def                        = converter< mark, Endianess >;
        using offset_def                      = converter< std::size_t, Endianess >;
        static constexpr std::size_t max_size = decl::max_size;
        using size_type                       = bounded< std::size_t, max_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, value_type item )
        {
                mark_def::serialize_at( buffer.first< mark_def::max_size >(), item.error_mark );
                offset_def::serialize_at( buffer.last< offset_def::max_size >(), item.offset );
                return size_type{};
        }

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> conversion_result< value_type >
        {
                auto [mused, mres] = mark_def::deserialize( buffer.first< mark_def::max_size >() );

                if ( std::holds_alternative< const mark* >( mres ) ) {
                        return { 0, *std::get_if< const mark* >( &mres ) };
                }

                auto [oused, ores] =
                    offset_def::deserialize( buffer.offset< mark_def::max_size >() );

                if ( std::holds_alternative< const mark* >( ores ) ) {
                        return { mused, *std::get_if< const mark* >( &ores ) };
                }
                return {
                    mused + oused,
                    error_record{
                        *std::get_if< 0 >( &mres ), *std::get_if< 0 >( &ores ) } };
        }
};

template < typename T, std::size_t N, endianess_enum Endianess >
struct converter< static_vector< T, N >, Endianess >
{
        using value_type = typename proto_traits< static_vector< T, N > >::value_type;

        static_assert( N <= std::numeric_limits< uint16_t >::max() );

        using counter_type = typename proto_traits< static_vector< T, N > >::counter_type;
        using counter_def  = converter< counter_type, Endianess >;
        using sub_def      = converter< T, Endianess >;

        static constexpr std::size_t counter_size = counter_def::max_size;

        static_assert( fixedly_sized< counter_type > );

        static constexpr std::size_t max_size = proto_traits< static_vector< T, N > >::max_size;
        static constexpr std::size_t min_size = counter_size;
        using size_type                       = bounded< std::size_t, min_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, value_type item )
        {

                counter_def::serialize_at(
                    buffer.template first< counter_size >(),
                    static_cast< counter_type >( item.size() ) );

                /// TODO: this duplicates std::array, generalize?
                auto iter = buffer.begin() + counter_size;
                for ( std::size_t i : range( item.size() ) ) {
                        std::span< uint8_t, sub_def::max_size > sub_view{ iter, sub_def::max_size };

                        bounded sub_bused = sub_def::serialize_at( sub_view, item[i] );

                        std::advance( iter, *sub_bused );
                }

                auto used = static_cast< std::size_t >( std::distance( buffer.begin(), iter ) );
                auto opt_bused = size_type::make( used );
                EMLABCPP_ASSERT( opt_bused );

                return *opt_bused;
        }

        static constexpr auto deserialize( bounded_view< const uint8_t*, size_type > buffer )
            -> conversion_result< value_type >
        {
                auto [cused, cres] =
                    counter_def::deserialize( buffer.template first< counter_size >() );

                if ( std::holds_alternative< const mark* >( cres ) ) {
                        return { cused, *std::get_if< const mark* >( &cres ) };
                }
                std::size_t cnt    = *std::get_if< 0 >( &cres );
                std::size_t offset = counter_size;
                value_type  res{};

                for ( std::size_t i : range( cnt ) ) {
                        std::ignore = i;

                        auto opt_view =
                            buffer.template opt_offset< typename sub_def::size_type >( offset );
                        if ( !opt_view ) {
                                return { offset, &SIZE_ERR };
                        }

                        auto [used, subres] = sub_def::deserialize( *opt_view );

                        offset += used;

                        if ( std::holds_alternative< const mark* >( subres ) ) {
                                return { offset, *std::get_if< const mark* >( &subres ) };
                        } else {
                                res.push_back( *std::get_if< 0 >( &subres ) );
                        }
                }

                return { offset, res };
        }
};

template < decomposable T, endianess_enum Endianess >
requires(
    !std::derived_from< T, converter_def_type_base > &&
    !quantity_derived< T > ) struct converter< T, Endianess >
{
        using decl                            = proto_traits< T >;
        using value_type                      = typename decl::value_type;
        static constexpr std::size_t max_size = decl::max_size;
        static constexpr std::size_t min_size = decl::min_size;

        using size_type = bounded< std::size_t, min_size, max_size >;

        using tuple_type = typename decl::tuple_type;
        using sub_def    = converter< tuple_type, Endianess >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                tuple_type tpl = decompose( item );
                return sub_def::serialize_at( buffer, tpl );
        }

        static constexpr auto deserialize( bounded_view< const uint8_t*, size_type > buffer )
            -> conversion_result< value_type >
        {

                auto [used, subres] = sub_def::deserialize( buffer );

                if ( std::holds_alternative< const mark* >( subres ) ) {
                        return { used, *std::get_if< const mark* >( &subres ) };
                } else {
                        return { used, compose< T >( *std::get_if< 0 >( &subres ) ) };
                }
        }
};

}  // namespace emlabcpp::protocol
