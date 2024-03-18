#pragma once

#include "emlabcpp/experimental/cfg/base.h"
#include "emlabcpp/protocol/converter.h"

#include <span>
#include <tuple>

namespace emlabcpp::cfg
{

template < std::endian Endianess, typename T, typename ChecksumFunction >
std::tuple< bool, std::span< std::byte > >
store_impl( std::span< std::byte > buffer, T const& item, ChecksumFunction&& chcksm_f )
{
        using sig_conv = protocol::converter_for< checksum, Endianess >;
        using conv     = protocol::converter_for< T, Endianess >;
        if ( buffer.size() < sig_conv::max_size + conv::max_size )
                return { false, buffer };
        std::span< std::byte > data = buffer.subspan< sig_conv::max_size, conv::max_size >();
        bounded const used  = conv::serialize_at( data.subspan< 0, conv::max_size >(), item );
        data                = data.first( *used );
        checksum const chck = chcksm_f( data );
        sig_conv::serialize_at( buffer.subspan< 0, sig_conv::max_size >(), chck );
        return { true, buffer.subspan( sig_conv::max_size + data.size() ) };
}

}  // namespace emlabcpp::cfg
