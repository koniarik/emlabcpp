/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
///

#pragma once

#include "./packet_handler.h"

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

        output_message serialize( output_value const& val )
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

                return match(
                    seq_.get_message(),
                    []( sequencer_read_request const to_read ) -> return_type {
                            return *to_read;
                    },
                    [&]( input_message const msg ) -> return_type {
                            return match( handler::extract( msg ), convert_to< return_type >{} );
                    } );
        }

private:
        sequencer_type seq_;
};

}  // namespace emlabcpp::protocol
