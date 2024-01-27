///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
/// and associated documentation files (the "Software"), to deal in the Software without
/// restriction, including without limitation the rights to use, copy, modify, merge, publish,
/// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or
/// substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
/// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
/// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#pragma once

#include "emlabcpp/either.h"
#include "emlabcpp/protocol/converter.h"

namespace emlabcpp::protocol
{

/// Handler for serialization and extraction of datatypes used by the register_map. This provides
/// interface for handling conversion of bytes to types used in the map. `serialize` and `extract`
/// works directly with the types used by the map, based on compile time key. `select` and `insert`
/// works with the map itself based on runtime information.
template < typename Map >
struct register_handler
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
                using def = converter_for< reg_def_type< Key >, Map::endianess >;

                message_type res( max_size );
                static_assert( def::size_type::max_val <= max_size );
                const bounded used = def::serialize_at(
                    std::span< std::byte, def::size_type::max_val >( res ), val );
                res.resize( *used );
                return res;
        }

        // TODO: what happens if the key is bad?
        static message_type select( const map_type& m, key_type key )
        {
                return m.with_register( key, [&]< typename RegType >( const RegType& reg ) {
                        return serialize< RegType::key >( reg.value );
                } );
        }

        template < key_type Key >
        static either< reg_value_type< Key >, error_record >
        extract( const view< const std::byte* >& msg )
        {
                using def = converter_for< reg_def_type< Key >, Map::endianess >;

                auto opt_view = bounded_view< const std::byte*, typename def::size_type >::make(
                    view_n( msg.begin(), std::min( def::max_size, msg.size() ) ) );
                if ( !opt_view )
                        return error_record{ SIZE_ERR, 0 };
                reg_value_type< Key > res;
                auto                  sres = def::deserialize( *opt_view, res );
                if ( sres.has_error() )
                        return error_record{ *sres.get_error(), sres.used };
                return res;
        }

        static std::optional< error_record >
        insert( map_type& m, key_type key, const view< const std::byte* >& buff )
        {
                std::optional< error_record > res;
                m.with_register( key, [&]< typename RegType >( RegType& reg ) {
                        extract< RegType::key >( buff ).match(
                            [&]( const auto& val ) {
                                    reg.value = val;
                            },
                            [&]( const auto& err ) {
                                    res = err;
                            } );
                } );
                return res;
        }
};

}  // namespace emlabcpp::protocol
