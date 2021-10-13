#include "emlabcpp/algorithm.h"
#include "emlabcpp/assert.h"
#include "emlabcpp/either.h"
#include "emlabcpp/iterators/numeric.h"
#include "emlabcpp/protocol/decl.h"
#include "emlabcpp/view.h"

#include <span>

#pragma once

namespace emlabcpp
{

template < typename, protocol_endianess_enum >
struct protocol_def;

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
&&requires( view< const uint8_t* > buff )
{
        {
                T::deserialize( buff )
                } -> std::same_as< either<
                    protocol_result< typename T::size_type, typename T::value_type >,
                    protocol_error_record > >;
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
                for ( std::size_t i : range( max_size ) ) {
                        bget( buffer, max_size - i - 1 ) = static_cast< uint8_t >( item & 0xFF );
                        item                             = static_cast< D >( item >> 8 );
                }
                return size_type{};
        }

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> either< protocol_result< size_type, value_type >, protocol_error_record >
        {
                if ( buffer.size() < max_size ) {
                        return protocol_error_record{ PROTOCOL_NS, LOWSIZE_ERR, 0 };
                }

                value_type res{};
                for ( std::size_t i : range( max_size ) ) {
                        res = static_cast< D >( res << 8 );
                        res = static_cast< D >( res | bget( buffer, i ) );
                }
                return protocol_result{ size_type{}, res };
        }
};

template < protocol_declarable D, std::size_t N, protocol_endianess_enum Endianess >
struct protocol_def< std::array< D, N >, Endianess >
{
        using value_type = typename protocol_decl< std::array< D, N > >::value_type;
        static constexpr std::size_t max_size = protocol_decl< std::array< D, N > >::max_size;

        using sub_def   = protocol_def< D, Endianess >;
        using size_type = bounded< std::size_t, sub_def::size_type::min_val * N, max_size >;

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

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> either< protocol_result< size_type, value_type >, protocol_error_record >
        {
                value_type                             res{};
                auto                                   iter = buffer.begin();
                std::optional< protocol_error_record > opt_err{};

                for ( std::size_t i : range( N ) ) {
                        view sub_buff{ iter, buffer.end() };

                        sub_def::deserialize( sub_buff )
                            .match(
                                [&]( auto sub_res ) {
                                        res[i] = sub_res.val;
                                        std::advance( iter, *sub_res.used );
                                },
                                [&]( protocol_error_record rec ) {
                                        rec.byte_index +=
                                            static_cast< uint16_t >( iter - buffer.begin() );
                                        opt_err = rec;
                                } );

                        if ( opt_err ) {
                                return *opt_err;
                        }
                }

                auto used      = std::distance( buffer.begin(), iter );
                auto opt_bused = size_type::make( used );
                EMLABCPP_ASSERT( opt_bused );
                return protocol_result{ *opt_bused, res };
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

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> either< protocol_result< size_type, value_type >, protocol_error_record >
        {
                value_type res;

                auto                                   iter = buffer.begin();
                std::optional< protocol_error_record > opt_err;

                until_index< sizeof...( Ds ) >( [&]< std::size_t i >() {
                        using D       = std::tuple_element_t< i, def_type >;
                        using sub_def = protocol_def< D, Endianess >;

                        sub_def::deserialize( view{ iter, buffer.end() } )
                            .match(
                                [&]( auto sub_res ) {
                                        std::get< i >( res ) = sub_res.val;
                                        std::advance( iter, *sub_res.used );
                                },
                                [&]( protocol_error_record rec ) {
                                        rec.byte_index +=
                                            static_cast< uint16_t >( iter - buffer.begin() );
                                        opt_err = rec;
                                } );

                        return opt_err.has_value();
                } );

                if ( opt_err ) {
                        return *opt_err;
                }

                auto used      = std::distance( buffer.begin(), iter );
                auto opt_bused = size_type::make( used );
                EMLABCPP_ASSERT( opt_bused );
                return protocol_result{ *opt_bused, res };
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
        static_assert( id_def::size_type::has_single_element );

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

        using result_either =
            either< protocol_result< size_type, value_type >, protocol_error_record >;

        static constexpr auto deserialize( const view< const uint8_t* >& buffer ) -> result_either
        {
                return id_def::deserialize( buffer ).bind_left( [&]( auto sub_res ) {
                        id_type id   = sub_res.val;
                        bounded used = sub_res.used;

                        result_either res = protocol_error_record{ PROTOCOL_NS, UNDEFVAR_ERR, 0 };

                        view item_view{ buffer.begin() + *used, buffer.end() };

                        until_index< sizeof...( Ds ) >( [&]< std::size_t i >() {
                                using D = std::variant_alternative_t< i, def_type >;

                                if ( id != i ) {
                                        return false;
                                }

                                protocol_def< D, Endianess >::deserialize( item_view )
                                    .match(
                                        [&]( auto sub_res ) {
                                                res = protocol_result< size_type, value_type >{
                                                    sub_res.used + bounded_constant< 1 >,
                                                    value_type{ sub_res.val } };
                                        },
                                        [&]( protocol_error_record rec ) {
                                                rec.byte_index += 1;
                                                res = rec;
                                        } );
                                return true;
                        } );

                        return res;
                } );
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

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> either< protocol_result< size_type, value_type >, protocol_error_record >
        {
                if ( buffer.size() < max_size ) {
                        return protocol_error_record{ PROTOCOL_NS, LOWSIZE_ERR, 0 };
                }
                std::bitset< N > res;
                for ( std::size_t i : range( max_size ) ) {
                        std::bitset< 8 > byte = bget( buffer, i );
                        for ( std::size_t j : range( 8u ) ) {
                                res[i * 8 + j] = byte[j];
                        }
                }
                return protocol_result{ size_type{}, res };
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

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> either< protocol_result< size_type, value_type >, protocol_error_record >
        {
                auto opt_msg = value_type::make( buffer );

                if ( !opt_msg ) {
                        return protocol_error_record{ PROTOCOL_NS, BIGSIZE_ERR, 0 };
                }

                auto opt_bused = size_type::make( opt_msg->size() );
                EMLABCPP_ASSERT( opt_bused );
                return protocol_result{ *opt_bused, *opt_msg };
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

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> either< protocol_result< size_type, value_type >, protocol_error_record >
        {
                return sub_def::deserialize( buffer ).convert_left( [&]( auto sub_res ) {
                        return protocol_result{
                            sub_res.used, static_cast< value_type >( sub_res.val - Offset ) };
                } );
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

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> either< protocol_result< size_type, value_type >, protocol_error_record >
        {
                return sub_def::deserialize( buffer ).convert_left( [&]( auto sub_res ) {
                        return protocol_result{ sub_res.used, value_type{ sub_res.val } };
                } );
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

        using result_either =
            either< protocol_result< size_type, value_type >, protocol_error_record >;

        static constexpr auto deserialize( const view< const uint8_t* >& buffer ) -> result_either
        {
                return sub_def::deserialize( buffer ).bind_left(
                    [&]( auto sub_res ) -> result_either {
                            auto opt_val = value_type::make( sub_res.val );
                            if ( !opt_val ) {
                                    return protocol_error_record{ PROTOCOL_NS, BOUNDS_ERR, 0 };
                            }
                            return protocol_result{ sub_res.used, *opt_val };
                    } );
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

        using counter_def       = protocol_def< CounterDef, Endianess >;
        using counter_size_type = typename counter_def::size_type;
        using counter_type      = typename counter_def::value_type;

        // we expect that counter item does not have dynamic size
        static_assert( counter_size_type::has_single_element );

        static constexpr std::size_t min_size = counter_def::max_size + sub_def::size_type::min_val;

        using size_type = bounded< std::size_t, min_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                auto vused =
                    sub_def::serialize_at( buffer.template last< sub_def::max_size >(), item );
                counter_def::serialize_at(
                    buffer.template first< counter_def::max_size >(),
                    static_cast< counter_type >( *vused ) );
                return vused + counter_size_type{};
        }

        using result_either =
            either< protocol_result< size_type, value_type >, protocol_error_record >;

        static constexpr auto deserialize( const view< const uint8_t* >& buffer ) -> result_either
        {
                return counter_def::deserialize( buffer ).bind_left(
                    [&]( auto sub_res ) -> result_either {
                            counter_type used       = sub_res.val;
                            auto         start_iter = buffer.begin() + counter_def::max_size;

                            if ( std::distance( start_iter, buffer.end() ) < used ) {
                                    return protocol_error_record{ PROTOCOL_NS, LOWSIZE_ERR, 0 };
                            }

                            return sub_def::deserialize( view_n( start_iter, used ) )
                                .convert_left( [&]( auto sub_res ) {
                                        return protocol_result{
                                            bounded_constant< counter_def::max_size > +
                                                sub_res.used,
                                            sub_res.val };
                                } )
                                .convert_right( [&]( protocol_error_record rec ) {
                                        rec.byte_index += counter_def::max_size;
                                        return rec;
                                } );
                    } );
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

        using return_either =
            either< protocol_result< size_type, value_type >, protocol_error_record >;

        static constexpr auto deserialize( const view< const uint8_t* >& buffer ) -> return_either
        {
                return sub_def::deserialize( buffer ).bind_left(
                    [&]( auto sub_res ) -> return_either {
                            if ( sub_res.val != V ) {
                                    return protocol_error_record{ PROTOCOL_NS, BADVAL_ERR, 0 };
                            }

                            return protocol_result{ sub_res.used, tag< V >{} };
                    } );
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

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> either< protocol_result< size_type, value_type >, protocol_error_record >
        {
                std::optional< protocol_result< size_type, value_type > > opt_res;

                until_index< sizeof...( Ds ) >( [&]< std::size_t i >() {
                        using sub_def =
                            protocol_def< std::variant_alternative_t< i, def_variant >, Endianess >;

                        sub_def::deserialize( buffer ).match(
                            [&]( auto sub_res ) {
                                    opt_res.emplace( sub_res.used, value_type{ sub_res.val } );
                            },
                            [&]( protocol_error_record ) {} );
                        return bool( opt_res );
                } );

                if ( opt_res ) {
                        return *opt_res;
                }

                return protocol_error_record{ PROTOCOL_NS, GROUP_ERR, 0 };
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

}  // namespace emlabcpp
