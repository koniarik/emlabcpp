#include "emlabcpp/protocol/packet_handler.h"

#pragma once

namespace emlabcpp::protocol
{

struct endpoint_load_error
{
};

using endpoint_error = std::variant< error_record, endpoint_load_error >;

template < typename InputPacket, typename OutputPacket >
class endpoint
{

public:
        using sequencer_type = typename InputPacket::sequencer_type;
        using output_message = typename OutputPacket::message_type;
        using input_message  = typename InputPacket::message_type;
        using output_value   = typename OutputPacket::value_type;
        using input_value    = typename InputPacket::value_type;

        output_message serialize( const output_value& val )
        {
                using handler = packet_handler< OutputPacket >;
                return handler::serialize( val );
        }

        template < typename Container >
        void insert( Container&& data )
        {
                seq_.insert( std::forward< Container >( data ) );
        }

        std::variant< std::size_t, input_value, error_record > get_value()
        {
                using return_type = std::variant< std::size_t, input_value, error_record >;
                using handler     = packet_handler< InputPacket >;

                return seq_.get_message()
                    .convert_left( convert_to< return_type >{} )
                    .convert_right( []( const input_message msg ) {
                            return handler::extract( msg )
                                .convert_left( convert_to< return_type >{} )
                                .convert_right( convert_to< return_type >{} )
                                .join();
                    } )
                    .join();
        }

private:
        sequencer_type seq_;
};

}  // namespace emlabcpp::protocol
