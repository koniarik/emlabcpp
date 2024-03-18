#pragma once

#include "emlabcpp/experimental/cfg/base.h"
#include "emlabcpp/protocol/converter.h"

#include <span>
#include <tuple>

namespace emlabcpp::cfg
{

template < typename T, std::endian Endianess, typename ChecksumFunction >
std::tuple< bool, T, std::span< std::byte > >
load_impl( std::span< std::byte > buffer, ChecksumFunction&& chcksm_f )
{
        using sig_conv = protocol::converter_for< checksum, Endianess >;
        using conv     = protocol::converter_for< T, Endianess >;

        T result{};

        checksum                          chcksm;
        protocol::conversion_result const sres = sig_conv::deserialize( buffer, chcksm );
        if ( sres.has_error() )
                return { false, result, buffer };

        protocol::conversion_result const cres =
            conv::deserialize( buffer.subspan( sig_conv::max_size ), result );

        std::span< std::byte > const data = buffer.subspan( sig_conv::max_size, cres.used );

        bool const chcksum_matches = chcksm_f( data ) == chcksm;
        return {
            !cres.has_error() && chcksum_matches,
            result,
            buffer.subspan( sig_conv::max_size + cres.used ) };
}

}  // namespace emlabcpp::cfg
