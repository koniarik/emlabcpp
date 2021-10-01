#include "emlabcpp/algorithm.h"
#include "emlabcpp/assert.h"
#include "emlabcpp/either.h"
#include "emlabcpp/iterators/numeric.h"
#include "emlabcpp/protocol/base.h"
#include "emlabcpp/view.h"

#include <span>

#pragma once

namespace emlabcpp
{

template < typename, protocol_endianess_enum >
struct protocol_item;

template < typename T >
concept protocol_item_check = requires()
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

template < protocol_base_type T, protocol_endianess_enum Endianess >
struct protocol_item< T, Endianess > : protocol_item_decl< T >
{
        using protocol_item_decl< T >::max_size;
        using typename protocol_item_decl< T >::value_type;

        static constexpr bool is_big_endian = Endianess == PROTOCOL_BIG_ENDIAN;

        using size_type = bounded< std::size_t, max_size, max_size >;

        static constexpr auto& bget( auto& buffer, std::size_t i )
        {
                return buffer[is_big_endian ? i : max_size - 1 - i];
        }

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, value_type item )
        {
                for ( std::size_t i : range( max_size ) ) {
                        bget( buffer, max_size - i - 1 ) = static_cast< uint8_t >( item & 0xFF );
                        item                             = static_cast< T >( item >> 8 );
                }
                return size_type{};
        }

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> either< protocol_result< size_type, value_type >, protocol_error_record >
        {
                if ( buffer.size() < max_size ) {
                        return protocol_error_record{ PROTOCOL_NS, LOWSIZE_ERR, 0 };
                }

                T res{};
                for ( std::size_t i : range( max_size ) ) {
                        res = static_cast< T >( res << 8 );
                        res = static_cast< T >( res | bget( buffer, i ) );
                }
                return protocol_result< size_type, value_type >{ size_type{}, res };
        }
};

template < protocol_itemizable T, std::size_t N, protocol_endianess_enum Endianess >
struct protocol_item< std::array< T, N >, Endianess > : protocol_item_decl< std::array< T, N > >
{
        using protocol_item_decl< std::array< T, N > >::max_size;
        using typename protocol_item_decl< std::array< T, N > >::value_type;

        using sub_item = protocol_item< T, Endianess >;

        using size_type = bounded< std::size_t, sub_item::size_type::min_val * N, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                auto iter = buffer.begin();
                for ( std::size_t i : range( N ) ) {

                        std::span< uint8_t, sub_item::max_size > sub_view{
                            iter, sub_item::max_size };

                        bounded bused = sub_item::serialize_at( sub_view, item[i] );

                        std::advance( iter, *bused );
                }
                auto used      = static_cast< std::size_t >( iter - buffer.begin() );
                auto opt_bused = size_type::make( used );
                EMLABCPP_ASSERT( opt_bused );
                return *opt_bused;
        }

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> either< protocol_result< size_type, value_type >, protocol_error_record >
        {
                std::array< T, N >                     res{};
                auto                                   iter = buffer.begin();
                std::optional< protocol_error_record > opt_err{};

                for ( std::size_t i : range( N ) ) {
                        view sub_buff{ iter, buffer.end() };
                        sub_item::deserialize( sub_buff )
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

                auto used      = static_cast< std::size_t >( iter - buffer.begin() );
                auto opt_bused = size_type::make( used );
                EMLABCPP_ASSERT( opt_bused );
                return protocol_result< size_type, value_type >{ *opt_bused, res };
        }
};

template < protocol_itemizable... Ts, protocol_endianess_enum Endianess >
struct protocol_item< std::tuple< Ts... >, Endianess > : protocol_item_decl< std::tuple< Ts... > >
{
        using protocol_item_decl< std::tuple< Ts... > >::max_size;
        using typename protocol_item_decl< std::tuple< Ts... > >::value_type;

        static constexpr std::size_t min_size =
            ( protocol_item< Ts, Endianess >::size_type::min_val + ... );

        using def_type  = std::tuple< Ts... >;
        using size_type = bounded< std::size_t, min_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                auto iter = buffer.begin();

                for_each_index< sizeof...( Ts ) >( [&]< std::size_t i >() {
                        using sub_item =
                            protocol_item< std::tuple_element_t< i, def_type >, Endianess >;

                        std::span< uint8_t, sub_item::max_size > sub_view{
                            iter, sub_item::max_size };

                        bounded bused = sub_item::serialize_at( sub_view, std::get< i >( item ) );

                        std::advance( iter, *bused );
                } );

                return *size_type::make( std::distance( buffer.begin(), iter ) );
        }

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> either< protocol_result< size_type, value_type >, protocol_error_record >
        {
                value_type res;

                auto                                   iter = buffer.begin();
                std::optional< protocol_error_record > opt_err;

                for_each_index< sizeof...( Ts ) >( [&]< std::size_t i >() {
                        if ( opt_err ) {
                                return;
                        }

                        using T        = std::tuple_element_t< i, def_type >;
                        using sub_item = protocol_item< T, Endianess >;

                        sub_item::deserialize( view{ iter, buffer.end() } )
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
                } );

                auto used = static_cast< std::size_t >( iter - buffer.begin() );
                if ( opt_err ) {
                        return { *opt_err };
                }

                auto opt_bused = size_type::make( used );
                EMLABCPP_ASSERT( opt_bused );
                return protocol_result< size_type, value_type >{ *opt_bused, res };
        }
};

template < protocol_itemizable... Ts, protocol_endianess_enum Endianess >
struct protocol_item< std::variant< Ts... >, Endianess >
  : protocol_item_decl< std::variant< Ts... > >
{
        using protocol_item_decl< std::variant< Ts... > >::max_size;
        using typename protocol_item_decl< std::variant< Ts... > >::value_type;

        using def_type = std::variant< Ts... >;
        using id_type  = uint8_t;
        using id_item  = protocol_item< id_type, Endianess >;

        static constexpr std::size_t id_size = id_item::max_size;

        static_assert( sizeof...( Ts ) < std::numeric_limits< id_type >::max() );

        static constexpr std::size_t min_size =
            id_item::size_type::min_val +
            std::min( { protocol_item< Ts, Endianess >::size_type::min_val... } );

        using size_type = bounded< std::size_t, min_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                id_item::serialize_at(
                    buffer.template first< id_size >(), static_cast< id_type >( item.index() ) );

                std::optional< size_type > opt_res;
                until_index< sizeof...( Ts ) >( [&]< std::size_t i >() {
                        if ( i != item.index() ) {
                                return false;
                        }
                        using subitem =
                            protocol_item< std::variant_alternative_t< i, def_type >, Endianess >;

                        // this also asserts that id has static serialized size
                        opt_res =
                            bounded_constant< id_item::max_size > +
                            subitem::serialize_at(
                                buffer.template subspan< id_item::max_size, subitem::max_size >(),
                                std::get< i >( item ) );

                        return true;
                } );
                EMLABCPP_ASSERT( opt_res );
                return *opt_res;
        }

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> either< protocol_result< size_type, value_type >, protocol_error_record >

        {
                return id_item::deserialize( buffer ).bind_left( [&]( auto sub_res ) {
                        id_type id   = sub_res.val;
                        bounded used = sub_res.used;

                        either< protocol_result< size_type, value_type >, protocol_error_record >
                            res = { protocol_error_record{ PROTOCOL_NS, UNDEFVAR_ERR, 0 } };

                        view item_view{ buffer.begin() + *used, buffer.end() };

                        until_index< sizeof...( Ts ) >( [&]< std::size_t i >() {
                                using T = std::variant_alternative_t< i, def_type >;

                                if ( id != i ) {
                                        return false;
                                }

                                protocol_item< T, Endianess >::deserialize( item_view )
                                    .match(
                                        [&]( auto sub_res ) {
                                                res = protocol_result< size_type, value_type >(
                                                    sub_res.used + bounded_constant< 1 >,
                                                    value_type{ sub_res.val } );
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
struct protocol_item< std::bitset< N >, Endianess > : protocol_item_decl< std::bitset< N > >
{
        using protocol_item_decl< std::bitset< N > >::max_size;
        using typename protocol_item_decl< std::bitset< N > >::value_type;

        static constexpr bool is_big_endian = Endianess == PROTOCOL_BIG_ENDIAN;

        using size_type = bounded< std::size_t, max_size, max_size >;

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
                        return { protocol_error_record{ PROTOCOL_NS, LOWSIZE_ERR, 0 } };
                }
                std::bitset< N > res;
                for ( std::size_t i : range( max_size ) ) {
                        std::bitset< 8 > byte = bget( buffer, i );
                        for ( std::size_t j : range( 8u ) ) {
                                res[i * 8 + j] = byte[j];
                        }
                }
                return protocol_result< size_type, value_type >{ size_type{}, res };
        }
};

template < std::size_t N, protocol_endianess_enum Endianess >
struct protocol_item< protocol_sizeless_message< N >, Endianess >
  : protocol_item_decl< protocol_sizeless_message< N > >
{
        using protocol_item_decl< protocol_sizeless_message< N > >::max_size;
        using typename protocol_item_decl< protocol_sizeless_message< N > >::value_type;

        using size_type = bounded< std::size_t, 0, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                for ( std::size_t i : range( item.size() ) ) {
                        buffer[i] = item[i];
                }
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
                return protocol_result< size_type, value_type >{ *opt_bused, *opt_msg };
        }
};

template < protocol_itemizable T, T Offset, protocol_endianess_enum Endianess >
struct protocol_item< protocol_offset< T, Offset >, Endianess >
  : protocol_item_decl< protocol_offset< T, Offset > >
{
        using protocol_item_decl< protocol_offset< T, Offset > >::max_size;
        using typename protocol_item_decl< protocol_offset< T, Offset > >::value_type;

        using sub_item  = protocol_item< T, Endianess >;
        using size_type = typename sub_item::size_type;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                return sub_item::serialize_at( buffer, item + Offset );
        }

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> either< protocol_result< size_type, value_type >, protocol_error_record >
        {
                return sub_item::deserialize( buffer ).convert_left(
                    [&]( auto sub_res ) -> protocol_result< size_type, value_type > {
                            return {
                                sub_res.used, static_cast< value_type >( sub_res.val - Offset ) };
                    } );
        }
};

template < quantity_derived T, protocol_endianess_enum Endianess >
struct protocol_item< T, Endianess > : protocol_item_decl< T >
{
        using protocol_item_decl< T >::max_size;
        using typename protocol_item_decl< T >::value_type;

        using inner_type = typename T::value_type;

        static_assert( protocol_itemizable< inner_type > );

        using sub_item  = protocol_item< inner_type, Endianess >;
        using size_type = typename sub_item::size_type;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                return sub_item::serialize_at( buffer, *item );
        }

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> either< protocol_result< size_type, value_type >, protocol_error_record >
        {
                return sub_item::deserialize( buffer ).convert_left(
                    [&]( protocol_result< size_type, inner_type > sub_res ) {
                            return protocol_result< size_type, value_type >{
                                sub_res.used, T{ sub_res.val } };
                    } );
        }
};

template < protocol_itemizable T, T Min, T Max, protocol_endianess_enum Endianess >
struct protocol_item< bounded< T, Min, Max >, Endianess >
  : protocol_item_decl< bounded< T, Min, Max > >
{
        using protocol_item_decl< bounded< T, Min, Max > >::max_size;
        using typename protocol_item_decl< bounded< T, Min, Max > >::value_type;

        using sub_item  = protocol_item< T, Endianess >;
        using size_type = typename sub_item::size_type;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                return sub_item::serialize_at( buffer, *item );
        }

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> either< protocol_result< size_type, value_type >, protocol_error_record >
        {
                return sub_item::deserialize( buffer ).bind_left(
                    [&]( auto sub_res ) -> either<
                                            protocol_result< size_type, value_type >,
                                            protocol_error_record > {
                            auto opt_val = value_type::make( sub_res.val );
                            if ( !opt_val ) {
                                    return protocol_error_record{ PROTOCOL_NS, BOUND_ERR, 0 };
                            }
                            return protocol_result< size_type, value_type >{
                                sub_res.used, *opt_val };
                    } );
        }
};

template <
    protocol_itemizable     CounterDef,
    protocol_itemizable     T,
    protocol_endianess_enum Endianess >
struct protocol_item< protocol_sized_buffer< CounterDef, T >, Endianess >
  : protocol_item_decl< protocol_sized_buffer< CounterDef, T > >
{
        using protocol_item_decl< protocol_sized_buffer< CounterDef, T > >::max_size;
        using typename protocol_item_decl< protocol_sized_buffer< CounterDef, T > >::value_type;

        using sub_item = protocol_item< T, Endianess >;

        using counter_item      = protocol_item< CounterDef, Endianess >;
        using counter_size_type = typename counter_item::size_type;
        using counter_type      = typename counter_item::value_type;

        // we expect that counter item does not have dynamic size
        static_assert( counter_size_type::has_single_element );

        static constexpr std::size_t min_size =
            counter_item::max_size + sub_item::size_type::min_val;

        using size_type = bounded< std::size_t, min_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                auto vused =
                    sub_item::serialize_at( buffer.template last< sub_item::max_size >(), item );
                counter_item::serialize_at(
                    buffer.template first< counter_item::max_size >(),
                    static_cast< counter_type >( *vused ) );
                return vused + counter_size_type{};
        }

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> either< protocol_result< size_type, value_type >, protocol_error_record >
        {
                return counter_item::deserialize( buffer ).bind_left(
                    [&]( protocol_result< counter_size_type, counter_type > sub_res )
                        -> either<
                            protocol_result< size_type, value_type >,
                            protocol_error_record > {
                            counter_type used       = sub_res.val;
                            auto         start_iter = buffer.begin() + counter_item::max_size;
                            if ( std::distance( start_iter, buffer.end() ) < used ) {
                                    return protocol_error_record{ PROTOCOL_NS, LOWSIZE_ERR, 0 };
                            }

                            return sub_item::deserialize( view_n( start_iter, used ) )
                                .convert_left( [&]( auto sub_res ) {
                                        return protocol_result< size_type, value_type >{
                                            bounded_constant< counter_item::max_size > +
                                                sub_res.used,
                                            sub_res.val };
                                } )
                                .convert_right( [&]( protocol_error_record rec ) {
                                        rec.byte_index += counter_item::max_size;
                                        return rec;
                                } );
                    } );
        }
};

template < auto V, protocol_endianess_enum Endianess >
struct protocol_item< tag< V >, Endianess > : protocol_item_decl< tag< V > >
{
        using protocol_item_decl< tag< V > >::max_size;
        using typename protocol_item_decl< tag< V > >::value_type;

        using sub_item = protocol_item< decltype( V ), Endianess >;

        using size_type = typename sub_item::size_type;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& )
        {
                return sub_item::serialize_at( buffer, V );
        }

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> either< protocol_result< size_type, value_type >, protocol_error_record >
        {
                return sub_item::deserialize( buffer ).bind_left(
                    [&]( auto sub_res ) -> either<
                                            protocol_result< size_type, value_type >,
                                            protocol_error_record > {
                            if ( sub_res.val != V ) {
                                    return protocol_error_record{ PROTOCOL_NS, BADVAL_ERR, 0 };
                            }

                            return protocol_result< size_type, value_type >{
                                sub_res.used, tag< V >{} };
                    } );
        }
};

template < typename... Ts, protocol_endianess_enum Endianess >
struct protocol_item< protocol_group< Ts... >, Endianess >
  : protocol_item_decl< protocol_group< Ts... > >
{
        using protocol_item_decl< protocol_group< Ts... > >::max_size;
        using typename protocol_item_decl< protocol_group< Ts... > >::value_type;

        static constexpr std::size_t min_size =
            std::min( { protocol_item< Ts, Endianess >::size_type::min_val... } );

        using def_variant = std::variant< Ts... >;
        using size_type   = bounded< std::size_t, min_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                std::optional< size_type > opt_res;
                until_index< sizeof...( Ts ) >( [&]< std::size_t i >() {
                        if ( i != item.index() ) {
                                return false;
                        }
                        using subitem = protocol_item<
                            std::variant_alternative_t< i, def_variant >,
                            Endianess >;
                        opt_res = subitem::serialize_at(
                            buffer.template subspan< 0, subitem::max_size >(),
                            std::get< i >( item ) );
                        return true;
                } );
                EMLABCPP_ASSERT( opt_res );
                return *opt_res;
        }

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> either< protocol_result< size_type, value_type >, protocol_error_record >
        {
                std::optional< protocol_result< size_type, value_type > > opt_res;

                until_index< sizeof...( Ts ) >( [&]< std::size_t i >() {
                        using sub_item = protocol_item<
                            std::variant_alternative_t< i, def_variant >,
                            Endianess >;

                        sub_item::deserialize( buffer ).match(
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

template < protocol_endianess_enum Endianess, typename T, protocol_endianess_enum ParentEndianess >
struct protocol_item< protocol_endianess< Endianess, T >, ParentEndianess >
  : protocol_item< T, Endianess >
{
};

}  // namespace emlabcpp
