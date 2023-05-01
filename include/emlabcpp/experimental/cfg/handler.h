#include "emlabcpp/experimental/cfg/store.h"

#pragma once

namespace emlabcpp::cfg
{

template < typename Payload, typename Field, std::endian Endianess >
struct handler
{
        using chcksum_conv = protocol::converter_for< checksum, Endianess >;

        template < typename ChecksumFunction >
        static bool store(
            std::span< std::byte > target_buffer,
            const Payload&         pl,
            view< const Field* >   fields,
            ChecksumFunction&&     chcksm_f )
        {
                bool                   success;
                std::span< std::byte > buffer = target_buffer;

                std::tie( success, buffer ) = store_impl< Endianess >(
                    buffer,
                    header{
                        .field_count = static_cast< uint32_t >( fields.size() ),
                    },
                    chcksm_f );
                if ( !success ) {
                        return false;
                }

                std::tie( success, buffer ) = store_impl< Endianess >( buffer, pl, chcksm_f );
                if ( !success ) {
                        return false;
                }

                for ( const Field& fp : fields ) {
                        std::tie( success, buffer ) =
                            store_impl< Endianess >( buffer, fp, chcksm_f );
                        if ( !success ) {
                                return false;
                        }
                }

                return success;
        }

        template < typename T, typename ChecksumFunction >
        static std::tuple< bool, T, std::span< std::byte > >
        load_impl( std::span< std::byte > buffer, ChecksumFunction&& chcksm_f )
        {
                using sig_conv = protocol::converter_for< checksum, Endianess >;
                using conv     = protocol::converter_for< T, Endianess >;
                checksum chcksm;
                sig_conv::deserialize( buffer, chcksm );

                T                           result;
                protocol::conversion_result cres =
                    conv::deserialize( buffer.subspan( sig_conv::max_size ), result );

                std::span< std::byte > data = buffer.subspan( sig_conv::max_size, cres.used );

                bool chcksum_matches = chcksm_f( data ) == chcksm;

                return {
                    !cres.has_error() && chcksum_matches,
                    result,
                    buffer.subspan( sig_conv::max_size + cres.used ) };
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
                std::tie( success, head, buffer ) = load_impl< header >( buffer, chcksm_f );
                if ( !success ) {
                        return load_result::DESERIALIZATION_ERROR;
                }

                Payload pl;
                std::tie( success, pl, buffer ) = load_impl< Payload >( buffer, chcksm_f );
                if ( !success ) {
                        return load_result::DESERIALIZATION_ERROR;
                }

                if ( !pl_f( std::as_const( pl ) ) ) {
                        return load_result::PAYLOAD_REFUSED;
                }

                for ( std::size_t i : range( head.field_count ) ) {
                        Field f;
                        std::tie( success, f, buffer ) = load_impl< Field >( buffer, chcksm_f );
                        if ( !success ) {
                                return load_result::DESERIALIZATION_ERROR;
                        }
                        field_f( std::as_const( f ) );
                }
                return load_result::SUCCESS;
        }
};

}  // namespace emlabcpp::cfg
