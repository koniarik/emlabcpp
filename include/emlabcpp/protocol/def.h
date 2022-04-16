#include "emlabcpp/algorithm.h"
#include "emlabcpp/assert.h"
#include "emlabcpp/bounded_view.h"
#include "emlabcpp/either.h"
#include "emlabcpp/iterators/numeric.h"
#include "emlabcpp/protocol/decl.h"
#include "emlabcpp/protocol/serializer.h"

#pragma once

namespace emlabcpp
{

// protocol_def<T,E> structure defines how type T should be serialized and deserialized. Each type
// or kind of types should overlead this structure and use same attributes as protocol_decl<T,E>. E
// is edianess of the serialization used.
template < typename, protocol_endianess_enum >
struct protocol_def;

// protocol_def_check<T> concept verifies that 'T' is valid overload of protocol_def. Use this in
// tests of custom protocol_def overloads.
template < typename T >
concept protocol_def_check = requires()
{
        {
                T::max_size
                } -> std::convertible_to< std::size_t >;
        typename T::value_type;
        bounded_derived< typename T::size_type >;
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
                } -> std::same_as< protocol_result< typename T::value_type > >;
};

template < protocol_base_type D, protocol_endianess_enum Endianess >
struct protocol_def< D, Endianess >
{
        using value_type                      = typename protocol_decl< D >::value_type;
        static constexpr std::size_t max_size = protocol_decl< D >::max_size;
        using size_type                       = bounded< std::size_t, max_size, max_size >;

        static constexpr bool is_big_endian = Endianess == PROTOCOL_BIG_ENDIAN;

        static constexpr auto& bget( auto& buffer, std::size_t i )
        {
                return buffer[is_big_endian ? i : max_size - 1 - i];
        }

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, value_type item )
        {
                protocol_serializer< value_type, Endianess >::serialize_at( buffer, item );
                return size_type{};
        }

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> protocol_result< value_type >
        {
                return {
                    max_size, protocol_serializer< value_type, Endianess >::deserialize( buffer ) };
        }
};

template < protocol_declarable D, std::size_t N, protocol_endianess_enum Endianess >
struct protocol_def< std::array< D, N >, Endianess >
{
        using value_type = typename protocol_decl< std::array< D, N > >::value_type;
        static constexpr std::size_t max_size = protocol_decl< std::array< D, N > >::max_size;

        using sub_def       = protocol_def< D, Endianess >;
        using sub_size_type = typename sub_def::size_type;
        using size_type     = bounded< std::size_t, sub_def::size_type::min_val * N, max_size >;

        // In both methods, we create the bounded size without properly checking that it the bounded
        // type was made properly (that is, that the provided std::size_t value is in the range).
        // Thas is ok as long as variant "we advanced the iter only by at max `sub_def::max_size`
        // `N` times".

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
            -> protocol_result< value_type >
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

                        if ( std::holds_alternative< const protocol_mark* >( subres ) ) {
                                return { offset, *std::get_if< const protocol_mark* >( &subres ) };
                        } else {
                                res[i] = *std::get_if< 0 >( &subres );
                        }
                }

                return protocol_result{ offset, res };
        }
};

template < protocol_declarable... Ds, protocol_endianess_enum Endianess >
struct protocol_def< std::tuple< Ds... >, Endianess >
{
        using def_type = std::tuple< Ds... >;

        using value_type                      = typename protocol_decl< def_type >::value_type;
        static constexpr std::size_t max_size = protocol_decl< def_type >::max_size;
        static constexpr std::size_t min_size =
            ( protocol_def< Ds, Endianess >::size_type::min_val + ... + 0 );
        using size_type = bounded< std::size_t, min_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                auto iter = buffer.begin();

                for_each_index< sizeof...( Ds ) >( [&]< std::size_t i >() {
                        using sub_def =
                            protocol_def< std::tuple_element_t< i, def_type >, Endianess >;

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
            -> protocol_result< value_type >
        {
                value_type res;

                std::size_t                           offset = 0;
                std::optional< const protocol_mark* > opt_err;

                until_index< sizeof...( Ds ) >( [&]< std::size_t i >() {
                        using D       = std::tuple_element_t< i, def_type >;
                        using sub_def = protocol_def< D, Endianess >;

                        auto opt_view =
                            buffer.template opt_offset< typename sub_def::size_type >( offset );

                        if ( !opt_view ) {
                                opt_err = &SIZE_ERR;
                                return true;
                        }

                        auto [sused, sres] = sub_def::deserialize( *opt_view );
                        offset += sused;
                        if ( std::holds_alternative< const protocol_mark* >( sres ) ) {
                                opt_err = *std::get_if< const protocol_mark* >( &sres );
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

template < protocol_declarable... Ds, protocol_endianess_enum Endianess >
struct protocol_def< std::variant< Ds... >, Endianess >
{
        using def_type                        = std::variant< Ds... >;
        using value_type                      = typename protocol_decl< def_type >::value_type;
        static constexpr std::size_t max_size = protocol_decl< def_type >::max_size;

        using id_type                        = uint8_t;
        using id_def                         = protocol_def< id_type, Endianess >;
        static constexpr std::size_t id_size = id_def::max_size;

        static_assert(
            sizeof...( Ds ) < std::numeric_limits< id_type >::max(),
            "Number of items for variant is limited by the size of one byte - 256 items" );
        static_assert( protocol_fixedly_sized< id_type > );

        static constexpr std::size_t min_size =
            id_def::size_type::min_val +
            std::min( { protocol_def< Ds, Endianess >::size_type::min_val... } );
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
                            protocol_def< std::variant_alternative_t< i, def_type >, Endianess >;

                        // this also asserts that id has static serialized size
                        opt_res =
                            bounded_constant< id_def::max_size > +
                            sub_def::serialize_at(
                                buffer.template subspan< id_def::max_size, sub_def::max_size >(),
                                std::get< i >( item ) );

                        return true;
                } );
                // The only case in which this can fire is that iteration 0...sizeof...(Ds) above
                // did not mathced item.index(), which should not be possible
                EMLABCPP_ASSERT( opt_res );
                return *opt_res;
        }

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> protocol_result< value_type >
        {
                protocol_result< value_type > res{ 0, &UNDEFVAR_ERR };

                auto [iused, idres] = id_def::deserialize( buffer.template first< id_size >() );
                std::size_t used    = iused;

                if ( std::holds_alternative< const protocol_mark* >( idres ) ) {
                        res.res = *std::get_if< const protocol_mark* >( &idres );
                        return res;
                }
                id_type id = std::get< 0 >( idres );

                auto item_view = buffer.template offset< id_size >();

                until_index< sizeof...( Ds ) >( [&]< std::size_t i >() {
                        using D       = std::variant_alternative_t< i, def_type >;
                        using sub_def = protocol_def< D, Endianess >;

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
                        if ( std::holds_alternative< const protocol_mark* >( sres ) ) {
                                res.res = *std::get_if< const protocol_mark* >( &sres );
                        } else {
                                res.res = value_type{ *std::get_if< 0 >( &sres ) };
                        }
                        return true;
                } );

                return res;
        }
};

template < std::size_t N, protocol_endianess_enum Endianess >
struct protocol_def< std::bitset< N >, Endianess >
{
        using value_type = typename protocol_decl< std::bitset< N > >::value_type;
        static constexpr std::size_t max_size = protocol_decl< std::bitset< N > >::max_size;
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
            -> protocol_result< value_type >
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

template < std::size_t N, protocol_endianess_enum Endianess >
struct protocol_def< protocol_sizeless_message< N >, Endianess >
{
        using value_type = typename protocol_decl< protocol_sizeless_message< N > >::value_type;
        static constexpr std::size_t max_size =
            protocol_decl< protocol_sizeless_message< N > >::max_size;
        using size_type = bounded< std::size_t, 0, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                for ( std::size_t i : range( item.size() ) ) {
                        buffer[i] = item[i];
                }
                // The size of protocol_size_message should always be within the 0...N range
                auto opt_bused = size_type::make( item.size() );
                EMLABCPP_ASSERT( opt_bused );
                return *opt_bused;
        }

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> protocol_result< value_type >
        {
                auto opt_msg = value_type::make( buffer );

                if ( !opt_msg ) {
                        return { 0, &BIGSIZE_ERR };
                }
                return protocol_result{ opt_msg->size(), *opt_msg };
        }
};

template < protocol_declarable D, auto Offset, protocol_endianess_enum Endianess >
struct protocol_def< protocol_offset< D, Offset >, Endianess >
{
        using value_type = typename protocol_decl< protocol_offset< D, Offset > >::value_type;
        static constexpr std::size_t max_size =
            protocol_decl< protocol_offset< D, Offset > >::max_size;

        using sub_def   = protocol_def< D, Endianess >;
        using size_type = typename sub_def::size_type;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                return sub_def::serialize_at( buffer, item + Offset );
        }

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> protocol_result< value_type >
        {
                auto [used, res] = sub_def::deserialize( buffer );
                if ( std::holds_alternative< const protocol_mark* >( res ) ) {
                        return { used, *std::get_if< const protocol_mark* >( &res ) };
                }
                return { used, static_cast< value_type >( *std::get_if< 0 >( &res ) - Offset ) };
        }
};

template < quantity_derived D, protocol_endianess_enum Endianess >
struct protocol_def< D, Endianess >
{
        using value_type                      = typename protocol_decl< D >::value_type;
        static constexpr std::size_t max_size = protocol_decl< D >::max_size;

        using inner_type = typename D::value_type;

        static_assert( protocol_declarable< inner_type > );

        using sub_def   = protocol_def< inner_type, Endianess >;
        using size_type = typename sub_def::size_type;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                return sub_def::serialize_at( buffer, *item );
        }

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> protocol_result< value_type >
        {
                auto [used, res] = sub_def::deserialize( buffer );
                if ( std::holds_alternative< const protocol_mark* >( res ) ) {
                        return { used, *std::get_if< const protocol_mark* >( &res ) };
                }
                return { used, value_type{ *std::get_if< 0 >( &res ) } };
        }
};

template < protocol_declarable D, D Min, D Max, protocol_endianess_enum Endianess >
struct protocol_def< bounded< D, Min, Max >, Endianess >
{
        using value_type = typename protocol_decl< bounded< D, Min, Max > >::value_type;
        static constexpr std::size_t max_size = protocol_decl< bounded< D, Min, Max > >::max_size;

        using sub_def   = protocol_def< D, Endianess >;
        using size_type = typename sub_def::size_type;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                return sub_def::serialize_at( buffer, *item );
        }

        using result_either = protocol_result< value_type >;

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> protocol_result< value_type >
        {
                auto [used, res] = sub_def::deserialize( buffer );
                if ( std::holds_alternative< const protocol_mark* >( res ) ) {
                        return { used, *std::get_if< const protocol_mark* >( &res ) };
                }
                auto opt_val = value_type::make( *std::get_if< 0 >( &res ) );
                if ( !opt_val ) {
                        return { 0, &BOUNDS_ERR };
                }
                return { used, *opt_val };
        }
};

template <
    protocol_declarable     CounterDef,
    protocol_declarable     D,
    protocol_endianess_enum Endianess >
struct protocol_def< protocol_sized_buffer< CounterDef, D >, Endianess >
{
        using value_type =
            typename protocol_decl< protocol_sized_buffer< CounterDef, D > >::value_type;
        static constexpr std::size_t max_size =
            protocol_decl< protocol_sized_buffer< CounterDef, D > >::max_size;

        using sub_def = protocol_def< D, Endianess >;

        using counter_def                         = protocol_def< CounterDef, Endianess >;
        using counter_size_type                   = typename counter_def::size_type;
        using counter_type                        = typename counter_def::value_type;
        static constexpr std::size_t counter_size = counter_def::max_size;

        // we expect that counter item does not have dynamic size
        static_assert( protocol_fixedly_sized< CounterDef > );

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
            -> protocol_result< value_type >
        {

                auto [cused, cres] =
                    counter_def::deserialize( buffer.template first< counter_size >() );

                if ( std::holds_alternative< const protocol_mark* >( cres ) ) {
                        return { cused, *std::get_if< const protocol_mark* >( &cres ) };
                }
                std::size_t used       = *std::get_if< 0 >( &cres );
                std::size_t size       = buffer.size() - counter_size;
                auto        start_iter = buffer.begin() + counter_size;

                if ( used > size ) {
                        return { cused, &SIZE_ERR };
                }

                auto opt_view = bounded_view< const uint8_t*, typename sub_def::size_type >::make(
                    view_n( start_iter, used ) );
                if ( !opt_view ) {
                        return { cused, &SIZE_ERR };
                }

                auto [sused, res] = sub_def::deserialize( *opt_view );
                return { cused + sused, res };
        }
};

template < auto V, protocol_endianess_enum Endianess >
struct protocol_def< tag< V >, Endianess >
{
        using value_type                      = typename protocol_decl< tag< V > >::value_type;
        static constexpr std::size_t max_size = protocol_decl< tag< V > >::max_size;

        using sub_def = protocol_def< decltype( V ), Endianess >;

        using size_type = typename sub_def::size_type;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& )
        {
                return sub_def::serialize_at( buffer, V );
        }

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> protocol_result< value_type >
        {
                auto [used, res] = sub_def::deserialize( buffer );
                if ( std::holds_alternative< const protocol_mark* >( res ) ) {
                        return { 0, *std::get_if< const protocol_mark* >( &res ) };
                }
                auto val = *std::get_if< 0 >( &res );
                if ( val != V ) {
                        return { 0, &BADVAL_ERR };
                }
                return { used, tag< V >{} };
        }
};

template < typename... Ds, protocol_endianess_enum Endianess >
struct protocol_def< protocol_group< Ds... >, Endianess >
{
        using value_type = typename protocol_decl< protocol_group< Ds... > >::value_type;
        static constexpr std::size_t max_size = protocol_decl< protocol_group< Ds... > >::max_size;
        static constexpr std::size_t min_size =
            std::min( { protocol_def< Ds, Endianess >::size_type::min_val... } );

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
                            protocol_def< std::variant_alternative_t< i, def_variant >, Endianess >;
                        opt_res = sub_def::serialize_at(
                            buffer.template subspan< 0, sub_def::max_size >(),
                            std::get< i >( item ) );
                        return true;
                } );
                // same check as for std::variant
                EMLABCPP_ASSERT( opt_res );
                return *opt_res;
        }

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> protocol_result< value_type >
        {
                std::size_t                                    offset = 0;
                std::size_t                                    size   = buffer.size();
                std::optional< protocol_result< value_type > > opt_res;

                until_index< sizeof...( Ds ) >( [&]< std::size_t i >() {
                        using sub_def =
                            protocol_def< std::variant_alternative_t< i, def_variant >, Endianess >;

                        // TODO: this pattern repeats mutliple times here
                        auto opt_view =
                            bounded_view< const uint8_t*, typename sub_def::size_type >::make(
                                view_n(
                                    buffer.begin() + offset,
                                    min( sub_def::max_size, size - offset ) ) );

                        if ( !opt_view ) {
                                return false;
                        }

                        auto [used, sres] = sub_def::deserialize( *opt_view );
                        if ( used == 0 ) {
                                return false;
                        }

                        if ( std::holds_alternative< const protocol_mark* >( sres ) ) {
                                opt_res.emplace(
                                    used, *std::get_if< const protocol_mark* >( &sres ) );
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

template < protocol_endianess_enum Endianess, typename D, protocol_endianess_enum ParentEndianess >
struct protocol_def< protocol_endianess< Endianess, D >, ParentEndianess >
  : protocol_def< D, Endianess >
{
};

template < std::derived_from< protocol_def_type_base > D, protocol_endianess_enum Endianess >
struct protocol_def< D, Endianess > : protocol_def< typename D::def_type, Endianess >
{
};

template < protocol_endianess_enum Endianess >
struct protocol_def< protocol_mark, Endianess >
{
        using value_type                      = typename protocol_decl< protocol_mark >::value_type;
        static constexpr std::size_t max_size = protocol_decl< protocol_mark >::max_size;
        using size_type                       = bounded< std::size_t, max_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, value_type item )
        {
                std::copy( item.begin(), item.end(), buffer.begin() );
                return size_type{};
        }

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> protocol_result< value_type >
        {
                value_type res{};
                std::copy( buffer.begin(), buffer.begin() + max_size, res.begin() );
                return { 0, res };
        }
};

template < protocol_endianess_enum Endianess >
struct protocol_def< protocol_error_record, Endianess >
{
        using decl                            = protocol_decl< protocol_error_record >;
        using value_type                      = typename decl::value_type;
        using mark_def                        = protocol_def< protocol_mark, Endianess >;
        using offset_def                      = protocol_def< std::size_t, Endianess >;
        static constexpr std::size_t max_size = decl::max_size;
        using size_type                       = bounded< std::size_t, max_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, value_type item )
        {
                mark_def::serialize_at( buffer.first< mark_def::max_size >(), item.mark );
                offset_def::serialize_at( buffer.last< offset_def::max_size >(), item.offset );
                return size_type{};
        }

        static constexpr auto deserialize( const bounded_view< const uint8_t*, size_type >& buffer )
            -> protocol_result< value_type >
        {
                auto [mused, mres] = mark_def::deserialize( buffer.first< mark_def::max_size >() );

                if ( std::holds_alternative< const protocol_mark* >( mres ) ) {
                        return { 0, *std::get_if< const protocol_mark* >( &mres ) };
                }

                auto [oused, ores] =
                    offset_def::deserialize( buffer.offset< mark_def::max_size >() );

                if ( std::holds_alternative< const protocol_mark* >( ores ) ) {
                        return { mused, *std::get_if< const protocol_mark* >( &ores ) };
                }
                return {
                    mused + oused,
                    protocol_error_record{
                        *std::get_if< 0 >( &mres ), *std::get_if< 0 >( &ores ) } };
        }
};

template < typename T, std::size_t N, protocol_endianess_enum Endianess >
struct protocol_def< static_vector< T, N >, Endianess >
{
        using value_type = typename protocol_decl< static_vector< T, N > >::value_type;

        static_assert( N <= std::numeric_limits< uint16_t >::max() );

        using counter_type = typename protocol_decl< static_vector< T, N > >::counter_type;
        using counter_def  = protocol_def< counter_type, Endianess >;
        using sub_def      = protocol_def< T, Endianess >;

        static constexpr std::size_t counter_size = counter_def::max_size;

        static_assert( protocol_fixedly_sized< counter_type > );

        static constexpr std::size_t max_size = protocol_decl< static_vector< T, N > >::max_size;
        static constexpr std::size_t min_size = counter_size;
        using size_type                       = bounded< std::size_t, min_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, value_type item )
        {

                counter_def::serialize_at(
                    buffer.template first< counter_size >(),
                    static_cast< counter_type >( item.size() ) );

                // TODO: this duplicates std::array, generalize?
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
            -> protocol_result< value_type >
        {
                auto [cused, cres] =
                    counter_def::deserialize( buffer.template first< counter_size >() );

                if ( std::holds_alternative< const protocol_mark* >( cres ) ) {
                        return { cused, *std::get_if< const protocol_mark* >( &cres ) };
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

                        if ( std::holds_alternative< const protocol_mark* >( subres ) ) {
                                return { offset, *std::get_if< const protocol_mark* >( &subres ) };
                        } else {
                                res.push_back( *std::get_if< 0 >( &subres ) );
                        }
                }

                return { offset, res };
        }
};

}  // namespace emlabcpp
