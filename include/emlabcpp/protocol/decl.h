#include "emlabcpp/protocol/base.h"
#include "emlabcpp/protocol/error.h"
#include "emlabcpp/protocol/message.h"
#include "emlabcpp/quantity.h"
#include "emlabcpp/static_vector.h"
#include "emlabcpp/types.h"

#include <variant>

#pragma once

namespace emlabcpp
{

// Each definition of item provided to protocol library should have specialization of
// 'protocol_decl' structure. This contains basic information of how it should be serialized.
template < typename D >
struct protocol_decl;

// This concepts limits types to types that can be declared, that is the overload of 'protocol_decl'
// is fully defined: protocol_decl::value_type contains definition of value produced by the
// declaration, protocol_decl::max_size contains estimated maximal size in bytes taken by the
// serialized value in the message.
template < typename D >
concept protocol_declarable = requires( D val )
{
        {
                protocol_decl< D >::max_size
                } -> std::convertible_to< std::size_t >;
        std::default_initializable< typename protocol_decl< D >::value_type >;
};

template < protocol_base_type D >
struct protocol_decl< D >
{
        using value_type                      = D;
        static constexpr std::size_t max_size = sizeof( D );
};

template < protocol_declarable D, std::size_t N >
struct protocol_decl< std::array< D, N > >
{
        using value_type                      = std::array< D, N >;
        static constexpr std::size_t max_size = protocol_decl< D >::max_size * N;
};

template < protocol_declarable... Ds >
struct protocol_decl< std::tuple< Ds... > >
{
        using value_type = std::tuple< typename protocol_decl< Ds >::value_type... >;
        static constexpr std::size_t max_size = ( protocol_decl< Ds >::max_size + ... + 0 );
};

template < protocol_declarable... Ds >
struct protocol_decl< std::variant< Ds... > >
{
        using id_type    = uint8_t;
        using id_decl    = protocol_decl< id_type >;
        using value_type = std::variant< typename protocol_decl< Ds >::value_type... >;

        static constexpr std::size_t max_size =
            id_decl::max_size + std::max< std::size_t >( { protocol_decl< Ds >::max_size... } );
};

template < std::size_t N >
struct protocol_decl< std::bitset< N > >
{
        using value_type = std::bitset< N >;

        static constexpr std::size_t max_size = ( N + 7 ) / 8;
};

template < std::size_t N >
struct protocol_decl< protocol_sizeless_message< N > >
{
        using value_type = protocol_sizeless_message< N >;

        static constexpr std::size_t max_size = N;
};

template < protocol_declarable D, auto Offset >
struct protocol_decl< protocol_offset< D, Offset > >
{
        using def_type   = typename protocol_offset< D, Offset >::def_type;
        using def_decl   = protocol_decl< def_type >;
        using value_type = typename def_decl::value_type;

        static constexpr std::size_t max_size = def_decl::max_size;
};

template < quantity_derived D >
struct protocol_decl< D >
{
        using value_type = D;

        static constexpr std::size_t max_size = protocol_decl< typename D::value_type >::max_size;
};

template < protocol_declarable D, D Min, D Max >
struct protocol_decl< bounded< D, Min, Max > >
{
        using value_type = bounded< D, Min, Max >;

        static constexpr std::size_t max_size = protocol_decl< D >::max_size;
};

template < protocol_declarable CounterDecl, protocol_declarable D >
struct protocol_decl< protocol_sized_buffer< CounterDecl, D > >
{
        using counter_decl = protocol_decl< CounterDecl >;
        using sub_decl     = protocol_decl< D >;
        using value_type   = typename sub_decl::value_type;

        static constexpr std::size_t max_size = counter_decl::max_size + sub_decl::max_size;
};

template < auto V >
struct protocol_decl< tag< V > >
{
        using sub_decl   = protocol_decl< decltype( V ) >;
        using value_type = tag< V >;

        static constexpr std::size_t max_size = sub_decl::max_size;
};

template < protocol_declarable... Ds >
struct protocol_decl< protocol_group< Ds... > >
{
        using value_type = std::variant< typename protocol_decl< Ds >::value_type... >;

        static constexpr std::size_t max_size =
            std::max< std::size_t >( { protocol_decl< Ds >::max_size..., 0 } );
};

template < protocol_endianess_enum Endianess, protocol_declarable D >
struct protocol_decl< protocol_endianess< Endianess, D > > : protocol_decl< D >
{
};

template < std::derived_from< protocol_def_type_base > D >
struct protocol_decl< D > : protocol_decl< typename D::def_type >
{
};

template <>
struct protocol_decl< protocol_mark >
{
        using value_type                      = protocol_mark;
        static constexpr std::size_t max_size = protocol_mark{}.max_size();
};

template <>
struct protocol_decl< protocol_error_record >
{
        using value_type = protocol_error_record;
        static constexpr std::size_t max_size =
            protocol_decl< protocol_mark >::max_size + protocol_decl< std::size_t >::max_size;
};

template < typename T, std::size_t N >
struct protocol_decl< static_vector< T, N > >
{
        using value_type   = static_vector< T, N >;
        using counter_type = uint16_t;
        static constexpr std::size_t max_size =
            protocol_decl< counter_type >::max_size + protocol_decl< T >::max_size * N;
};

}  // namespace emlabcpp
