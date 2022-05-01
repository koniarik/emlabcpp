#include "emlabcpp/protocol/sequencer.h"
#include "emlabcpp/protocol/serializer.h"
#include "emlabcpp/protocol/tuple.h"

#pragma once

namespace emlabcpp
{

template < typename T >
concept protocol_packet_def = requires( T t )
{
        {
                T::endianess
                } -> std::convertible_to< protocol_endianess_enum >;
        is_std_array_v< std::decay_t< decltype( T::prefix ) > >;
        protocol_declarable< typename T::size_type >;
        protocol_declarable< typename T::checksum_type >;
};

template < typename Def, typename Payload >
using protocol_packet_base = protocol_tuple<
    Def::endianess,
    std::decay_t< decltype( Def::prefix ) >,
    typename Def::size_type,
    Payload,
    typename Def::checksum_type >;

template < protocol_packet_def Def, typename Payload >
struct protocol_packet : protocol_packet_base< Def, Payload >
{
        using payload_type = Payload;
        using base         = protocol_packet_base< Def, Payload >;

        static constexpr auto prefix    = Def::prefix;
        static constexpr auto endianess = Def::endianess;

        using prefix_type                        = std::decay_t< decltype( prefix ) >;
        using prefix_decl                        = protocol_decl< prefix_type >;
        static constexpr std::size_t prefix_size = prefix_decl::max_size;

        using size_type                        = typename Def::size_type;
        using size_decl                        = protocol_decl< size_type >;
        static constexpr std::size_t size_size = size_decl::max_size;

        using payload_decl = protocol_decl< Payload >;
        using value_type   = typename payload_decl::value_type;

        using checksum_type = typename Def::checksum_type;
        using checksum_decl = protocol_decl< checksum_type >;

        static_assert( protocol_fixedly_sized< prefix_type > );
        static_assert( protocol_fixedly_sized< size_type > );

        struct sequencer_def
        {
                using message_type = typename base::message_type;
                using serializer   = protocol_serializer< size_type, endianess >;

                static constexpr std::array< uint8_t, prefix_decl::max_size > prefix = Def::prefix;
                static constexpr std::size_t fixed_size = prefix_size + size_decl::max_size;

                static constexpr std::size_t get_size( const auto& buffer )
                {
                        std::array< uint8_t, size_size > tmp;
                        std::copy_n( buffer.begin() + prefix_size, size_size, tmp.begin() );
                        return serializer::deserialize( tmp ) + prefix_size + size_size;
                }
        };

        using sequencer = protocol_sequencer< sequencer_def >;

        static constexpr checksum_type get_checksum( const view< const uint8_t* > mview )
        {
                return Def::get_checksum( mview );
        }
};

}  // namespace emlabcpp
