#include "emlabcpp/algorithm.h"
#include "emlabcpp/assert.h"
#include "emlabcpp/iterators/numeric.h"
#include "emlabcpp/protocol/base.h"
#include "emlabcpp/protocol/message.h"
#include "emlabcpp/quantity.h"
#include "emlabcpp/view.h"

#include <bitset>
#include <span>
#include <variant>

#pragma once

namespace emlabcpp
{

template < typename, protocol_endianess_enum >
struct protocol_item;

template < typename T, protocol_endianess_enum ParentEndianess >
using protocol_subitem = protocol_item<
    T,
    ( protocol_endianess< T > == protocol_endianess_enum::PARENT ? ParentEndianess :
                                                                   protocol_endianess< T > ) >;

template < typename T >
concept protocol_itemizable = std::default_initializable< T > && requires( T val )
{
        protocol_item< T, protocol_endianess_enum::BIG >{};
        std::same_as<
            typename protocol_item< T, protocol_endianess_enum::BIG >::size_type,
            typename protocol_item< T, protocol_endianess_enum::LITTLE >::size_type >;
        // TODO: ^^ this check may have more sense somewhere else
};

template < typename T >
concept protocol_item_check = requires()
{
        {
                T::max_size
                } -> std::convertible_to< std::size_t >;
        typename T::value_type;
        bounded_derived< typename T::size_type >;
}
&&requires( std::span< uint8_t, T::max_size > buff, T::value_type item )
{
        {
                T::serialize_at( buff, item )
                } -> std::same_as< typename T::size_type >;
}
&&requires( view< const uint8_t* > buff )
{
        {
                T::deserialize( buff )
                } -> std::same_as< std::variant<
                    protocol_result< typename T::size_type, typename T::value_type >,
                    protocol_error_record > >;
};

template < protocol_base_type T, protocol_endianess_enum Endianess >
struct protocol_item< T, Endianess >
{
        static constexpr std::size_t max_size      = sizeof( T );
        static constexpr bool        is_big_endian = Endianess == protocol_endianess_enum::BIG;

        using value_type = T;
        using size_type  = bounded< std::size_t, max_size, max_size >;

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
                return {};
        }

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> std::variant< protocol_result< size_type, value_type >, protocol_error_record >
        {
                if ( buffer.size() < max_size ) {
                        return { protocol_error_record{ PROTOCOL_NS, LOWSIZE_ERR, 0 } };
                }

                T res{};
                for ( std::size_t i : range( max_size ) ) {
                        res = static_cast< T >( res << 8 );
                        res = static_cast< T >( res | bget( buffer, i ) );
                }
                return { protocol_result{ size_type{}, res } };
        }
};

template < protocol_itemizable T, std::size_t N, protocol_endianess_enum Endianess >
struct protocol_item< std::array< T, N >, Endianess >
{
        using sub_item = protocol_subitem< T, Endianess >;

        static constexpr std::size_t max_size = sub_item::max_size * N;

        using value_type = std::array< T, N >;
        using size_type  = bounded< std::size_t, sub_item::size_type::min_val * N, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                auto iter = buffer.begin();
                for ( std::size_t i : range( N ) ) {

                        std::span< uint8_t, sub_item::max_size > sub_view{
                            iter, sub_item::max_size };

                        auto bused = sub_item::serialize_at( sub_view, item[i] );

                        std::advance( iter, *bused );
                }
                std::size_t used      = static_cast< std::size_t >( iter - buffer.begin() );
                auto        opt_bused = size_type::make( used );
                EMLABCPP_ASSERT( opt_bused );
                return *opt_bused;
        }

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> std::variant< protocol_result< size_type, value_type >, protocol_error_record >
        {
                std::array< T, N > res;
                auto               iter = buffer.begin();

                for ( std::size_t i : range( N ) ) {
                        view sub_buff{ iter, buffer.end() };
                        auto var = sub_item::deserialize( sub_buff );

                        if ( std::holds_alternative< protocol_error_record >( var ) ) {
                                auto rec = std::get< protocol_error_record >( var );
                                rec.byte_index += static_cast< uint16_t >( iter - buffer.begin() );
                                return { rec };
                        }
                        auto [used, val] = std::get< 0 >( var );
                        std::advance( iter, *used );
                        res[i] = val;
                }

                std::size_t used      = static_cast< std::size_t >( iter - buffer.begin() );
                auto        opt_bused = size_type::make( used );
                EMLABCPP_ASSERT( opt_bused );
                return { protocol_result< size_type, value_type >{ *opt_bused, res } };
        }
};

template < protocol_itemizable... Ts, protocol_endianess_enum Endianess >
struct protocol_item< std::tuple< Ts... >, Endianess >
{

        using value_type = std::tuple< Ts... >;

        static constexpr std::size_t max_size =
            ( protocol_subitem< Ts, Endianess >::max_size + ... );
        static constexpr std::size_t min_size =
            ( protocol_subitem< Ts, Endianess >::size_type::min_val + ... );

        using size_type = bounded< std::size_t, min_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                auto iter = buffer.begin();

                for_each( make_sequence_tuple_t< sizeof...( Ts ) >{}, [&]( auto index ) {
                        static constexpr std::size_t i = decltype( index )::value;
                        using sub_item =
                            protocol_subitem< std::tuple_element_t< i, value_type >, Endianess >;

                        std::span< uint8_t, sub_item::max_size > sub_view{
                            iter, sub_item::max_size };

                        auto bused = sub_item::serialize_at( sub_view, std::get< i >( item ) );

                        std::advance( iter, *bused );
                } );

                return *size_type::make( std::distance( buffer.begin(), iter ) );
        }

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> std::variant< protocol_result< size_type, value_type >, protocol_error_record >
        {
                value_type res;

                auto                                   iter = buffer.begin();
                std::optional< protocol_error_record > opt_err;

                for_each( make_sequence_tuple_t< sizeof...( Ts ) >{}, [&]( auto index ) {
                        if ( opt_err ) {
                                return;
                        }

                        static constexpr std::size_t i = decltype( index )::value;

                        using T        = std::tuple_element_t< i, value_type >;
                        using sub_item = protocol_subitem< T, Endianess >;

                        auto var = sub_item::deserialize( view{ iter, buffer.end() } );

                        if ( std::holds_alternative< protocol_error_record >( var ) ) {
                                opt_err = std::get< protocol_error_record >( var );
                                opt_err->byte_index +=
                                    static_cast< uint16_t >( iter - buffer.begin() );
                        } else {
                                auto [used, val] = std::get< 0 >( var );
                                std::advance( iter, *used );
                                std::get< i >( res ) = val;
                        }
                } );

                std::size_t used = static_cast< std::size_t >( iter - buffer.begin() );
                if ( opt_err ) {
                        return { *opt_err };
                }

                auto opt_bused = size_type::make( used );
                EMLABCPP_ASSERT( opt_bused );
                return protocol_result{ *opt_bused, res };
        }
};

template < protocol_itemizable... Ts, protocol_endianess_enum Endianess >
struct protocol_item< std::variant< Ts... >, Endianess >
{

        using id_type = uint8_t;
        using id_item = protocol_subitem< id_type, Endianess >;

        static_assert( sizeof...( Ts ) < std::numeric_limits< id_type >::max() );

        static constexpr std::size_t max_size =
            id_item::max_size + std::max( { protocol_subitem< Ts, Endianess >::max_size... } );
        static constexpr std::size_t min_size =
            id_item::size_type::min_val +
            std::min( { protocol_subitem< Ts, Endianess >::size_type::min_val... } );

        using value_type = std::variant< Ts... >;
        using size_type  = bounded< std::size_t, min_size, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                // TODO: maybe we should do serialization of id properly?
                buffer[0] = static_cast< id_type >( item.index() );

                return std::visit(
                    [&]( const auto& val ) -> size_type {
                            using T       = std::decay_t< decltype( val ) >;
                            using subitem = protocol_subitem< T, Endianess >;

                            // this also asserts that id has static serialized size
                            return bounded_constant< id_item::max_size > +
                                   subitem::serialize_at(
                                       buffer.template subspan<
                                           id_item::max_size,
                                           subitem::max_size >(),
                                       val );
                    },
                    item );
        }

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> std::variant< protocol_result< size_type, value_type >, protocol_error_record >

        {
                std::variant< protocol_result< size_type, value_type >, protocol_error_record >
                    res = { protocol_error_record{ PROTOCOL_NS, UNDEFVAR_ERR, 0 } };

                for_each( make_sequence_tuple_t< sizeof...( Ts ) >{}, [&]( auto integ_const ) {
                        using ic = decltype( integ_const );
                        using T  = std::variant_alternative_t< ic::value, value_type >;

                        if ( buffer[0] != ic::value ) {
                                return;
                        }

                        auto tmp = protocol_subitem< T, Endianess >::deserialize( tail( buffer ) );

                        if ( !std::holds_alternative< protocol_error_record >( tmp ) ) {
                                auto sub_res = std::get< 0 >( tmp );
                                res          = { protocol_result< size_type, value_type >{
                                    sub_res.used + bounded_constant< 1 >,
                                    value_type{ sub_res.val } } };
                        } else {
                                auto sub_err = std::get< 1 >( tmp );
                                sub_err.byte_index += 1;
                                res = { sub_err };
                        }
                } );

                return res;
        }
};

template < std::size_t N, protocol_endianess_enum Endianess >
struct protocol_item< std::bitset< N >, Endianess >
{
        static constexpr std::size_t max_size      = ( N + 7 ) / 8;
        static constexpr bool        is_big_endian = Endianess == protocol_endianess_enum::BIG;

        using value_type = std::bitset< N >;
        using size_type  = bounded< std::size_t, max_size, max_size >;

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
                return {};
        }

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> std::variant< protocol_result< size_type, value_type >, protocol_error_record >
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
                return { protocol_result{ size_type{}, res } };
        }
};

template < std::size_t N, protocol_endianess_enum Endianess >
struct protocol_item< protocol_sizeless_message< N >, Endianess >
{
        static constexpr std::size_t max_size = N;

        using value_type = protocol_sizeless_message< N >;
        using size_type  = bounded< std::size_t, 0, max_size >;

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
            -> std::variant< protocol_result< size_type, value_type >, protocol_error_record >
        {
                auto opt_msg = value_type::make( buffer );

                if ( !opt_msg ) {
                        return { protocol_error_record{ PROTOCOL_NS, BIGSIZE_ERR, 0 } };
                }

                auto opt_bused = size_type::make( opt_msg->size() );
                EMLABCPP_ASSERT( opt_bused );
                return { protocol_result{ *opt_bused, *opt_msg } };
        }
};

template < typename Tag, protocol_itemizable T, T Offset, protocol_endianess_enum Endianess >
struct protocol_item< protocol_offset< Tag, T, Offset >, Endianess >
{
        using sub_item = protocol_subitem< T, Endianess >;

        static constexpr std::size_t max_size = sub_item::max_size;

        using value_type = T;
        using size_type  = sub_item::size_type;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                return sub_item::serialize_at( buffer, item + Offset );
        }
        // TODO: maybe this class could check sanity more?
        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> std::variant< protocol_result< size_type, value_type >, protocol_error_record >
        {
                auto var = sub_item::deserialize( buffer );
                if ( std::holds_alternative< protocol_error_record >( var ) ) {
                        return { std::get< 1 >( var ) };
                }
                auto sub_res = std::get< 0 >( var );
                return { protocol_result{
                    sub_res.used, static_cast< value_type >( sub_res.val - Offset ) } };
        }
};

template < quantity_derived T, protocol_endianess_enum Endianess >
struct protocol_item< T, Endianess >
{

        using value_type = T;
        using inner_type = typename T::value_type;

        static_assert( protocol_itemizable< inner_type > );

        using sub_item = protocol_subitem< inner_type, Endianess >;

        static constexpr std::size_t max_size = sub_item::max_size;

        using size_type = sub_item::size_type;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                return sub_item::serialize_at( buffer, *item );
        }

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> std::variant< protocol_result< size_type, value_type >, protocol_error_record >
        {
                auto var = sub_item::deserialize( buffer );
                if ( std::holds_alternative< protocol_error_record >( var ) ) {
                        return { std::get< 1 >( var ) };
                }
                return {
                    protocol_result{ std::get< 0 >( var ).used, T{ std::get< 0 >( var ).val } } };
        }
};

template < protocol_itemizable T, T Min, T Max, protocol_endianess_enum Endianess >
struct protocol_item< bounded< T, Min, Max >, Endianess >
{

        using value_type = bounded< T, Min, Max >;
        using sub_item   = protocol_subitem< T, Endianess >;

        static constexpr std::size_t max_size = sub_item::max_size;

        using size_type = sub_item::size_type;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                return sub_item::serialize_at( buffer, *item );
        }

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> std::variant< protocol_result< size_type, value_type >, protocol_error_record >
        {
                auto var = sub_item::deserialize( buffer );
                if ( std::holds_alternative< protocol_error_record >( var ) ) {
                        return { std::get< 1 >( var ) };
                }
                auto opt_val = value_type::make( std::get< 0 >( var ).val );
                if ( !opt_val ) {
                        return { protocol_error_record{ PROTOCOL_NS, BOUND_ERR, 0 } };
                }
                return { protocol_result{ std::get< 0 >( var ).used, *opt_val } };
        }
};

template <
    protocol_itemizable     CounterType,
    protocol_itemizable     T,
    protocol_endianess_enum Endianess >
struct protocol_item< protocol_sized_buffer< CounterType, T >, Endianess >
{
        // TODO: make special case of this for bounded
        using value_type        = T;
        using counter_item      = protocol_subitem< CounterType, Endianess >;
        using counter_size_type = typename counter_item::size_type;

        // we expect that counter item does not have dynamic size
        static_assert( counter_size_type::has_single_element );

        using sub_item                        = protocol_subitem< T, Endianess >;
        static constexpr std::size_t max_size = counter_item::max_size + sub_item::max_size;

        using size_type =
            bounded< std::size_t, counter_item::max_size + sub_item::size_type::min_val, max_size >;

        static constexpr size_type
        serialize_at( std::span< uint8_t, max_size > buffer, const value_type& item )
        {
                auto vused =
                    sub_item::serialize_at( buffer.template last< sub_item::max_size >(), item );
                // TODO the static cast here may be a bad idea?
                counter_item::serialize_at(
                    buffer.template first< counter_item::max_size >(),
                    static_cast< CounterType >( *vused ) );
                return vused + counter_size_type{};
        }

        static constexpr auto deserialize( const view< const uint8_t* >& buffer )
            -> std::variant< protocol_result< size_type, value_type >, protocol_error_record >
        {
                auto cvar = counter_item::deserialize( buffer );
                if ( std::holds_alternative< protocol_error_record >( cvar ) ) {
                        return { std::get< 1 >( cvar ) };
                }
                CounterType used = std::get< 0 >( cvar ).val;

                auto start_iter = buffer.begin() + counter_item::max_size;
                if ( std::distance( start_iter, buffer.end() ) < used ) {
                        return { protocol_error_record{ PROTOCOL_NS, LOWSIZE_ERR, 0 } };
                }
                // TODO: code duplication under this, repeates in most overloads
                auto var = sub_item::deserialize( view_n( start_iter, used ) );
                if ( std::holds_alternative< protocol_error_record >( var ) ) {
                        return { std::get< 1 >( var ) };
                }
                return { protocol_result{
                    std::get< 0 >( cvar ).used + std::get< 0 >( var ).used,
                    T{ std::get< 0 >( var ).val } } };
        }
};

}  // namespace emlabcpp
