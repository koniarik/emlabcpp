#include "emlabcpp/experimental/cfg/store.h"

#pragma once

namespace emlabcpp::cfg
{

template < typename HeaderPayload, typename Field >
struct handler
{
        using header_payload = HeaderPayload;
        using field          = Field;
        using sig_handler    = protocol::handler< signature >;

        template < typename ChecksumFunction >
        static checksum signature_checksum(
            record_id          id,
            checksum           header_chck,
            checksum           fields_chck,
            ChecksumFunction&& chcksm_f )
        {

                using data         = std::tuple< record_id, checksum, checksum >;
                using data_handler = protocol::handler< data >;

                protocol::message data_data =
                    data_handler::serialize( data{ id, header_chck, fields_chck } );

                return chcksm_f( data_data );
        }

        template < typename ChecksumFunction >
        static bool store(
            std::span< std::byte > target_buffer,
            uint32_t               id,
            const HeaderPayload&   pl,
            view< const Field* >   fields,
            ChecksumFunction&&     chcksm_f )
        {

                bool                   success;
                checksum               header_chck;
                std::span< std::byte > buffer = target_buffer;

                std::tie( success, header_chck, buffer ) = store_impl(
                    buffer.subspan( sig_handler::max_size ),
                    header< HeaderPayload >{
                        .pl          = pl,
                        .field_count = static_cast< uint32_t >( fields.size() ),
                    },
                    chcksm_f );
                if ( !success ) {
                        return false;
                }

                checksum fields_chck = 0;
                for ( const Field& fp : fields ) {
                        checksum sub_chck;
                        std::tie( success, sub_chck, buffer ) = store_impl( buffer, fp, chcksm_f );
                        if ( !success ) {
                                return false;
                        }
                        fields_chck ^= sub_chck;
                }

                checksum fin;
                std::tie( success, fin, buffer ) = store_impl(
                    target_buffer,
                    signature{
                        .id          = id,
                        .header_chck = header_chck,
                        .fields_chck = fields_chck,
                        .sig_chck    = signature_checksum( id, header_chck, fields_chck, chcksm_f ),
                    },
                    chcksm_f );

                return true;
        }
};

}  // namespace emlabcpp::cfg
