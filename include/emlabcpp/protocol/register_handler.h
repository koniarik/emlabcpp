// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//
//  Copyright © 2022 Jan Veverak Koniarik
//  This file is part of project: emlabcpp
//
#include "emlabcpp/either.h"
#include "emlabcpp/protocol/def.h"

#pragma once

namespace emlabcpp::protocol
{

/// Handler for serialization and extraction of datatypes used by the register_map. This provides
/// interface for handling conversion of bytes to types used in the map. `serialize` and `extract`
/// works directly with the types used by the map, based on compile time key. `select` and `insert`
/// works with the map itself based on runtime information.
template < typename Map >
struct protocol_register_handler
{
        using map_type = Map;
        using key_type = typename map_type::key_type;

        static constexpr std::size_t max_size = map_type::max_value_size;

        using message_type = typename map_type::message_type;

        template < key_type Key >
        using reg_value_type = typename map_type::template reg_value_type< Key >;

        template < key_type Key >
        using reg_def_type = typename map_type::template reg_def_type< Key >;

        template < key_type Key >
        static message_type serialize( reg_value_type< Key > val )
        {
                using def = converter< reg_def_type< Key >, Map::endianess >;

                std::array< uint8_t, max_size > buffer;
                static_assert( def::max_size <= max_size );
                bounded used = def::serialize_at(
                    std::span< uint8_t, def::max_size >( buffer.begin(), def::max_size ), val );
                EMLABCPP_ASSERT( *used <= max_size );

                return *message_type::make( view_n( buffer.begin(), *used ) );
        }

        static message_type select( const map_type& m, key_type key )
        {
                return m.with_register( key, [&]< typename reg_type >( const reg_type& reg ) {
                        return serialize< reg_type::key >( reg.value );
                } );
        }

        template < key_type Key >
        static either< reg_value_type< Key >, protocol_error_record >
        extract( const view< const uint8_t* >& msg )
        {
                using def = converter< reg_def_type< Key >, Map::endianess >;

                auto opt_view = bounded_view< const uint8_t*, typename def::size_type >::make(
                    view_n( msg.begin(), std::min( def::max_size, msg.size() ) ) );
                if ( !opt_view ) {
                        return protocol_error_record{ SIZE_ERR, 0 };
                }
                auto [used, res] = def::deserialize( *opt_view );
                if ( std::holds_alternative< const protocol_mark* >( res ) ) {
                        return protocol_error_record{ *std::get< 1 >( res ), used };
                }
                return std::get< 0 >( res );
        }

        static std::optional< protocol_error_record >
        insert( map_type& m, key_type key, const view< const uint8_t* >& buff )
        {
                std::optional< protocol_error_record > res;
                m.setup_register( key, [&]< typename reg_type >() {
                        return extract< reg_type::key >( buff )
                            .convert_right( [&]( auto err ) {
                                    res = err;
                                    return m.template get_val< reg_type::key >();
                            } )
                            .join();
                } );
                return res;
        }
};

}  // namespace emlabcpp::protocol
