
#include "emlabcpp/experimental/cfg/base.h"
#include "emlabcpp/protocol/converter.h"

#include <span>
#include <tuple>

#pragma once

namespace emlabcpp::cfg
{

template < typename T, std::endian Endianess, typename ChecksumFunction >
std::tuple< bool, T, std::span< std::byte > >
load_impl( std::span< std::byte > buffer, ChecksumFunction&& chcksm_f )
{
        using sig_conv = protocol::converter_for< checksum, Endianess >;
        using conv     = protocol::converter_for< T, Endianess >;
        checksum chcksm;
        sig_conv::deserialize( buffer, chcksm );

        T                           result{};
        protocol::conversion_result cres =
            conv::deserialize( buffer.subspan( sig_conv::max_size ), result );

        std::span< std::byte > data = buffer.subspan( sig_conv::max_size, cres.used );

        bool chcksum_matches = chcksm_f( data ) == chcksm;

        return {
            !cres.has_error() && chcksum_matches,
            result,
            buffer.subspan( sig_conv::max_size + cres.used ) };
}

}  // namespace emlabcpp::cfg
