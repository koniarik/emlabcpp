
#include "emlabcpp/experimental/cfg/base.h"
#include "emlabcpp/protocol/handler.h"

#include <span>
#include <tuple>

#pragma once

namespace emlabcpp::cfg
{

template < typename T, typename ChecksumFunction >
std::tuple< bool, checksum, std::span< std::byte > >
store_impl( std::span< std::byte > buffer, const T& item, ChecksumFunction&& chcksm_f )
{
        using handler          = protocol::handler< T >;
        protocol::message data = handler::serialize( item );
        checksum          chck = chcksm_f( data );
        if ( data.size() > buffer.size() ) {
                return { false, chck, buffer };
        }

        std::copy_n( data.data(), data.size(), buffer.data() );
        return { true, chck, buffer.subspan( data.size() ) };
}

}  // namespace emlabcpp::cfg
