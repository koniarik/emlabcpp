#pragma once

#include "emlabcpp/experimental/cfg/load.h"
#include "emlabcpp/experimental/cfg/store.h"

namespace emlabcpp::cfg
{

template < typename Payload, typename Field, std::endian Endianess >
struct handler
{
        using chcksum_conv = protocol::converter_for< checksum, Endianess >;

        template < typename ChecksumFunction >
        static std::tuple< bool, std::span< std::byte > > store(
            std::span< std::byte > target_buffer,
            const Payload&         pl,
            view< const Field* >   fields,
            ChecksumFunction&&     chcksm_f )
        {
                return store(
                    target_buffer,
                    pl,
                    fields.size(),
                    [&]( std::size_t i ) {
                            return fields[i];
                    },
                    chcksm_f );
        }

        template < typename FieldFunction, typename ChecksumFunction >
        static std::tuple< bool, std::span< std::byte > > store(
            std::span< std::byte > target_buffer,
            const Payload&         pl,
            std::size_t            field_count,
            FieldFunction&&        field_f,
            ChecksumFunction&&     chcksm_f )
        {
                bool                   success;
                std::span< std::byte > buffer = target_buffer;

                std::tie( success, buffer ) = store_impl< Endianess >(
                    buffer,
                    header{
                        .field_count = static_cast< uint32_t >( field_count ),
                    },
                    chcksm_f );
                if ( !success ) {
                        return { false, {} };
                }
                std::tie( success, buffer ) = store_impl< Endianess >( buffer, pl, chcksm_f );
                if ( !success ) {
                        return { false, {} };
                }

                for ( const std::size_t i : range( field_count ) ) {
                        const Field fp = field_f( i );
                        std::tie( success, buffer ) =
                            store_impl< Endianess >( buffer, fp, chcksm_f );
                        if ( !success ) {
                                return { false, {} };
                        }
                }

                return { success, target_buffer.subspan( 0, buffer.size() ) };
        }

        template < typename PayloadFunction, typename FieldFunction, typename ChecksumFunction >
        static load_result load(
            std::span< std::byte > source,
            PayloadFunction&&      pl_f,
            FieldFunction&&        field_f,
            ChecksumFunction&&     chcksm_f )
        {
                std::span< std::byte > buffer = source;
                bool                   success;

                header head;
                std::tie( success, head, buffer ) =
                    load_impl< header, Endianess >( buffer, chcksm_f );
                if ( !success ) {
                        EMLABCPP_DEBUG_LOG( "Failed to deserialize header" );
                        return load_result::DESERIALIZATION_ERROR;
                }

                Payload pl;
                std::tie( success, pl, buffer ) =
                    load_impl< Payload, Endianess >( buffer, chcksm_f );
                if ( !success ) {
                        EMLABCPP_DEBUG_LOG( "Failed to deserialize payload" );
                        return load_result::DESERIALIZATION_ERROR;
                }

                if ( !pl_f( std::as_const( pl ) ) ) {
                        return load_result::PAYLOAD_REFUSED;
                }

                for ( const std::size_t i : range( head.field_count ) ) {
                        std::ignore = i;
                        Field f;
                        std::tie( success, f, buffer ) =
                            load_impl< Field, Endianess >( buffer, chcksm_f );
                        if ( !success ) {
                                EMLABCPP_DEBUG_LOG( "Failed to deserialize subitem" );
                                return load_result::DESERIALIZATION_ERROR;
                        }
                        field_f( std::as_const( f ) );
                }
                return load_result::SUCCESS;
        }
};

}  // namespace emlabcpp::cfg
