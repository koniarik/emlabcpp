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
                seq_.insert( data );
        }

        std::variant< std::size_t, input_value, error_record > get_message()
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

        template < typename ReadCallback >
        either< input_value, endpoint_error >
        load_variant( const std::size_t read_limit, ReadCallback&& read )
        {
                // TODO: duplication of sequencer_simple_load
                // TODO: decompose all of this /o
                // TODO: error reporting/propagation

                std::size_t                    to_read = sequencer_type::fixed_size;
                std::optional< input_message > opt_msg;
                std::size_t                    count = 0;
                while ( !opt_msg && count < read_limit ) {
                        std::optional data = read( to_read );
                        if ( !data ) {
                                return endpoint_error{ endpoint_load_error{} };
                        }
                        seq_.insert( *data );
                        seq_.get_message().match(
                            [&to_read, &count]( const sequencer_read_request read_req ) {
                                    to_read = *read_req;
                                    count   = 0;
                            },
                            [&opt_msg]( const input_message msg ) {
                                    opt_msg = msg;
                            } );

                        count += 1;
                }

                if ( !opt_msg ) {
                        return endpoint_error{ endpoint_load_error{} };
                }

                return extract_value( *opt_msg );
        }

private:
        either< input_value, endpoint_error > extract_value( const input_message& msg )
        {

                using handler = packet_handler< InputPacket >;

                return handler::extract( msg ).convert_right(
                    []( const error_record rec ) -> endpoint_error {
                            std::ignore = rec;
                            EMLABCPP_LOG( "Protocol error from endpoint: " << rec );
                            return endpoint_error{ rec };
                    } );
        }

        sequencer_type seq_;
};

}  // namespace emlabcpp::protocol
