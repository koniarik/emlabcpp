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
&&requires( bounded_view< const uint8_t*, typename T::size_type > buff )
{
        T::deserialize( buff );
};

template < typename Conv >
concept erroring_converter =
    requires( const bounded_view< const uint8_t*, typename Conv::size_type >& buffer )
{
        {
                Conv::deserialize( buffer )
                } -> std::same_as<
                    conversion_result< typename Conv::value_type, error_possibility::POSSIBLE > >;
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

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> conversion_result< value_type, error_possibility::IMPOSSIBLE >
        {
                return { max_size, serializer< value_type, Endianess >::deserialize( buffer ) };
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

        static constexpr error_possibility error_posib =
            ( fixedly_sized< D > && !erroring_converter< sub_converter > ) ?
                error_possibility::IMPOSSIBLE :
                error_possibility::POSSIBLE;

        static constexpr auto deserialize( bounded_view< const uint8_t*, size_type > buffer )
            -> conversion_result< value_type, error_posib >
        {

                value_type  res{};
                std::size_t offset = 0;

                for ( const std::size_t i : range( N ) ) {
                        auto opt_view = buffer.template opt_offset< sub_size_type >( offset );

                        if constexpr ( error_posib == error_possibility::POSSIBLE ) {
                                if ( !opt_view ) {
                                        return { offset, &SIZE_ERR };
                                }
                        }

                        auto sres = sub_converter::deserialize( *opt_view );
                        offset += sres.used;
                        if constexpr ( erroring_converter< sub_converter > ) {
                                if ( sres.has_error() ) {
                                        return { offset, sres.get_error() };
                                }
                        }

                        res[i] = *sres.get_value();
                }

                return { offset, res };
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

        static constexpr error_possibility error_posib =
            fixedly_sized< def_type > &&
                    ( !erroring_converter< converter< Ds, Endianess > > && ... && true ) ?
                error_possibility::IMPOSSIBLE :
                error_possibility::POSSIBLE;

        template < std::size_t I >
        using nth_converter = converter_for< std::tuple_element_t< I, def_type >, Endianess >;

        template < std::size_t I >
        using nth_size = typename nth_converter< I >::size_type;

        static constexpr auto deserialize( bounded_view< const uint8_t*, size_type > buffer )
            -> conversion_result< value_type, error_posib >
        {
                value_type res;

                std::size_t                  offset = 0;
                std::optional< const mark* > opt_err;

                until_index< sizeof...( Ds ) >(
                    [&offset, &opt_err, &res, &buffer]< std::size_t i >() {
                            auto opt_view = buffer.template opt_offset< nth_size< i > >( offset );

                            if constexpr ( !fixedly_sized< def_type > ) {
                                    if ( !opt_view ) {
                                            opt_err = &SIZE_ERR;
                                            return true;
                                    }
                            }

                            auto sres = nth_converter< i >::deserialize( *opt_view );
                            offset += sres.used;
                            if constexpr ( erroring_converter< nth_converter< i > > ) {
                                    if ( sres.has_error() ) {
                                            opt_err = sres.get_error();
                                            return true;
                                    }
                            }

                            std::get< i >( res ) = *sres.get_value();

                            return false;
                    } );

                if constexpr ( error_posib == error_possibility::POSSIBLE ) {
                        if ( opt_err ) {
                                return { offset, *opt_err };
                        }
                }
                return { offset, res };
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
        using id_def                         = converter_for< id_type, Endianess >;
        static constexpr std::size_t id_size = id_def::max_size;

        static_assert(
            sizeof...( Ds ) < std::numeric_limits< id_type >::max(),
            "Number of items for variant is limited by the size of one byte - 256 items" );
        static_assert( fixedly_sized< id_type > );
        using size_type = bounded< std::size_t, min_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                id_def::serialize_at(
                    buffer.template first< id_size >(), static_cast< id_type >( item.index() ) );

                return visit_index(
                    [&buffer, &item]< std::size_t i >() -> size_type {
                            using sub_converter = converter_for<
                                std::variant_alternative_t< i, def_type >,
                                Endianess >;

                            /// this also asserts that id has static serialized size
                            return bounded_constant< id_def::max_size > +
                                   sub_converter::serialize_at(
                                       buffer.template subspan<
                                           id_def::max_size,
                                           sub_converter::max_size >(),
                                       std::get< i >( item ) );
                    },
                    item );
        }

        template < std::size_t I >
        using nth_converter = converter_for< std::variant_alternative_t< I, def_type >, Endianess >;

        template < std::size_t I >
        using nth_size = typename nth_converter< I >::size_type;

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> conversion_result< value_type >
        {
                return id_def::deserialize( buffer.template first< id_size >() )
                    .bind_value( [&buffer]( const std::size_t iused, const id_type id ) {
                            auto item_view = buffer.template offset< id_size >();

                            conversion_result< value_type > res{ 0, &UNDEFVAR_ERR };
                            until_index< sizeof...( Ds ) >(
                                [&res, &item_view, &id, &iused]< std::size_t i >() {
                                        if ( id != i ) {
                                                return false;
                                        }

                                        auto opt_view =
                                            item_view.template opt_offset< nth_size< i > >( 0 );

                                        if ( !opt_view ) {
                                                res.res = &SIZE_ERR;
                                                return true;
                                        }

                                        res = conversion_result< value_type >{
                                            nth_converter< i >::deserialize( *opt_view )
                                                .convert_value( []( auto item ) {
                                                        return value_type{
                                                            std::in_place_index< i >, item };
                                                } ) };
                                        res.used += iused;
                                        return true;
                                } );

                            return res;
                    } );
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

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& )
            -> conversion_result< value_type >
        {
                return { 0, std::monostate{} };
        }
};

template < convertible T, std::endian Endianess >
struct converter< std::optional< T >, Endianess >
{
        using decl                            = traits_for< std::optional< T > >;
        using value_type                      = std::optional< T >;
        static constexpr std::size_t max_size = decl::max_size;
        static constexpr std::size_t min_size = decl::min_size;
        using presence_type                   = typename decl::presence_type;

        static_assert( fixedly_sized< presence_type > );

        using presence_def                         = converter_for< presence_type, Endianess >;
        static constexpr std::size_t presence_size = presence_def::max_size;
        using sub_converter                        = converter_for< T, Endianess >;
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

                return psize +
                       sub_converter::serialize_at(
                           buffer.template subspan< presence_size, sub_converter::max_size >(),
                           *opt_val );
        }

        static constexpr auto deserialize( bounded_view< const uint8_t*, size_type > buffer )
            -> conversion_result< value_type >
        {
                return presence_def::deserialize( buffer.template first< presence_size >() )
                    .bind_value(
                        [&buffer]( std::size_t pused, presence_type is_present )
                            -> conversion_result< value_type > {
                                if ( is_present == 0 ) {
                                        return { pused, value_type{} };
                                }
                                if ( is_present != 1 ) {
                                        return { pused, &BADVAL_ERR };
                                }

                                using sub_size = typename sub_converter::size_type;
                                auto opt_view =
                                    buffer.template opt_offset< sub_size >( presence_size );

                                if ( !opt_view ) {
                                        return { pused, &BADVAL_ERR };
                                }

                                auto res = sub_converter::deserialize( *opt_view )
                                               .convert_value( []( T item ) {
                                                       return std::make_optional( item );
                                               } );
                                res.used += pused;
                                return conversion_result< value_type >{ std::move( res ) };
                        } );
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

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> conversion_result< value_type >
        {
                std::bitset< N > res;
                for ( const std::size_t i : range( max_size ) ) {
                        std::bitset< 8 > byte = bget( buffer, i );
                        for ( const std::size_t j : range( 8u ) ) {
                                res[i * 8 + j] = byte[j];
                        }
                }
                return { max_size, res };
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

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
        {
                return sub_converter::deserialize( buffer ).convert_value(
                    []( D val ) -> value_type {
                            return static_cast< value_type >( val - Offset );
                    } );
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

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
        {
                return sub_converter::deserialize( buffer ).convert_value( []( inner_type val ) {
                        return value_type{ val };
                } );
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

        using result_type = conversion_result< value_type >;

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> conversion_result< value_type >
        {
                return sub_converter::deserialize( buffer ).bind_value(
                    []( std::size_t used, D item ) -> result_type {
                            auto opt_val = value_type::make( item );
                            if ( !opt_val ) {
                                    return { 0, &BOUNDS_ERR };
                            }
                            return { used, *opt_val };
                    } );
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

        using counter_def                         = converter_for< CounterDef, Endianess >;
        using counter_size_type                   = typename counter_def::size_type;
        using counter_type                        = typename counter_def::value_type;
        static constexpr std::size_t counter_size = counter_def::max_size;

        /// we expect that counter item does not have dynamic size
        static_assert( fixedly_sized< CounterDef > );

        using size_type = bounded< std::size_t, min_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                auto vused = sub_converter::serialize_at(
                    buffer.template last< sub_converter::max_size >(), item );
                auto cused = counter_def::serialize_at(
                    buffer.template first< counter_size >(),
                    static_cast< counter_type >( *vused ) );
                return vused + cused;
        }

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> conversion_result< value_type >
        {
                return counter_def::deserialize( buffer.template first< counter_size >() )
                    .bind_value(
                        [&buffer]( std::size_t cused, std::size_t buffer_size )
                            -> conversion_result< value_type > {
                                auto start_iter = buffer.begin() + counter_size;

                                auto opt_view = bounded_view<
                                    const uint8_t*,
                                    typename sub_converter::size_type >::
                                    make( view_n( start_iter, buffer_size ) );
                                if ( !opt_view ) {
                                        return { cused, &SIZE_ERR };
                                }

                                auto [sused, res] = sub_converter::deserialize( *opt_view );
                                return { cused + sused, res };
                        } );
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

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> conversion_result< value_type >
        {
                return sub_converter::deserialize( buffer ).bind_value(
                    []( std::size_t used, auto val ) -> conversion_result< value_type > {
                            if ( val != V ) {
                                    return { 0, &BADVAL_ERR };
                            }
                            return { used, tag< V >{} };
                    } );
        }
};

template < typename... Ds, std::endian Endianess >
struct converter< tag_group< Ds... >, Endianess >
{
        using decl                            = traits_for< tag_group< Ds... > >;
        using value_type                      = typename decl::value_type;
        static constexpr std::size_t max_size = decl::max_size;
        static constexpr std::size_t min_size = decl::min_size;

        using def_variant   = std::variant< Ds... >;
        using size_type     = bounded< std::size_t, min_size, max_size >;
        using sub_type      = typename decl::sub_type;
        using sub_converter = converter_for< sub_type, Endianess >;

        using sub_value = typename sub_converter::value_type;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                return visit_index(
                    [&buffer, &item]< std::size_t i >() {
                            using D = std::variant_alternative_t< i, def_variant >;

                            return sub_converter::serialize_at(
                                buffer.template subspan< 0, sub_converter::max_size >(),
                                std::make_tuple( tag< D::id >{}, std::get< i >( item ) ) );
                    },
                    item );
        }

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> conversion_result< value_type >
        {
                return sub_converter::deserialize( buffer ).convert_value( []( sub_value var ) {
                        return visit_index(
                            [&var]< std::size_t i >() {
                                    const auto* ptr = std::get_if< i >( &var );
                                    return value_type{
                                        std::in_place_index< i >, std::get< 1 >( *ptr ) };
                            },
                            var );
                } );
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

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> conversion_result< value_type >
        {
                std::size_t                                      offset = 0;
                const std::size_t                                size   = buffer.size();
                std::optional< conversion_result< value_type > > opt_res;

                const bool got_match = until_index< sizeof...(
                    Ds ) >( [&buffer, &offset, &size, &opt_res]< std::size_t i >() {
                        using sub_converter = converter_for<
                            std::variant_alternative_t< i, def_variant >,
                            Endianess >;

                        /// TODO: this pattern repeats mutliple times here
                        auto opt_view =
                            bounded_view< const uint8_t*, typename sub_converter::size_type >::make(
                                view_n(
                                    buffer.begin() + offset,
                                    std::min( sub_converter::max_size, size - offset ) ) );

                        if ( !opt_view ) {
                                return false;
                        }
                        opt_res = conversion_result< value_type >{
                            sub_converter::deserialize( *opt_view ).convert_value( []( auto item ) {
                                    return value_type{ item };
                            } ) };

                        if ( opt_res->used == 0 ) {
                                return false;
                        }
                        return true;
                } );

                if ( got_match ) {
                        return *opt_res;
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

template < std::endian Endianess >
struct converter< error_record, Endianess >
{
        using decl                            = traits_for< error_record >;
        using value_type                      = typename decl::value_type;
        using mark_def                        = converter_for< mark, Endianess >;
        using offset_def                      = converter_for< std::size_t, Endianess >;
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
                return mark_def::deserialize( buffer.first< mark_def::max_size >() )
                    .bind_value(
                        [&buffer]( const std::size_t mused, const mark m )
                            -> conversion_result< value_type > {
                                return offset_def::deserialize(
                                           buffer.offset< mark_def::max_size >() )
                                    .bind_value(
                                        [mused,
                                         &m]( const std::size_t oused, const std::size_t oval )
                                            -> conversion_result< value_type > {
                                                return { mused + oused, error_record{ m, oval } };
                                        } );
                        } );
        }
};

template < typename T, std::size_t N, std::endian Endianess >
struct converter< static_vector< T, N >, Endianess >
{
        using value_type = typename traits_for< static_vector< T, N > >::value_type;

        static_assert( N <= std::numeric_limits< uint16_t >::max() );

        using counter_type  = typename traits_for< static_vector< T, N > >::counter_type;
        using counter_def   = converter_for< counter_type, Endianess >;
        using sub_converter = converter_for< T, Endianess >;

        static constexpr std::size_t counter_size = counter_def::max_size;

        static_assert( fixedly_sized< counter_type > );

        static constexpr std::size_t max_size = traits_for< static_vector< T, N > >::max_size;
        static constexpr std::size_t min_size = traits_for< static_vector< T, N > >::min_size;
        using size_type                       = bounded< std::size_t, min_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, value_type item )
        {

                counter_def::serialize_at(
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

        static constexpr auto deserialize( bounded_view< const uint8_t*, size_type > buffer )
            -> conversion_result< value_type >
        {
                return counter_def::deserialize( buffer.template first< counter_size >() )
                    .bind_value(
                        [&buffer]( const std::size_t cused, const std::size_t count )
                            -> conversion_result< value_type > {
                                std::size_t offset = cused;
                                value_type  res{};

                                for ( const std::size_t i : range( count ) ) {
                                        std::ignore = i;

                                        auto opt_view = buffer.template opt_offset<
                                            typename sub_converter::size_type >( offset );
                                        if ( !opt_view ) {
                                                return { offset, &SIZE_ERR };
                                        }

                                        auto sres = sub_converter::deserialize( *opt_view );

                                        offset += sres.used;

                                        if constexpr ( erroring_converter< sub_converter > ) {
                                                if ( sres.has_error() ) {
                                                        return { offset, sres.get_error() };
                                                }
                                        }

                                        res.push_back( *sres.get_value() );
                                }

                                return { offset, res };
                        } );
        }
};

template < decomposable T, std::endian Endianess >
struct backup_converter< T, Endianess >
{
        using decl                            = traits_for< T >;
        using value_type                      = typename decl::value_type;
        static constexpr std::size_t max_size = decl::max_size;
        static constexpr std::size_t min_size = decl::min_size;

        using size_type = bounded< std::size_t, min_size, max_size >;

        using tuple_type    = typename decl::tuple_type;
        using sub_converter = converter_for< tuple_type, Endianess >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                tuple_type tpl = decompose( item );
                return sub_converter::serialize_at( buffer, tpl );
        }

        static constexpr auto deserialize( bounded_view< const uint8_t*, size_type > buffer )
        {
                return sub_converter::deserialize( buffer ).convert_value(
                    []( const tuple_type& val ) {
                            return compose< T >( val );
                    } );
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

        using sub_type      = std::array< uint8_t, sizeof( value_type ) >;
        using sub_converter = converter_for< sub_type, Endianess >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, value_type item )
        {
                sub_type sub;
                std::memcpy( &sub, &item, sizeof( value_type ) );
                return sub_converter::serialize_at( buffer, sub );
        }

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
        {
                return sub_converter::deserialize( buffer ).convert_value( []( sub_type val ) {
                        value_type res;
                        std::memcpy( &res, &val, sizeof( value_type ) );
                        return res;
                } );
        }
};

}  // namespace emlabcpp::protocol
