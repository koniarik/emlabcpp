#include "emlabcpp/either.h"
#include "emlabcpp/protocol/def.h"

#pragma once

namespace emlabcpp
{

// Handler for serialization and extraction of datatypes used by the register_map. This provides
// interface for handling conversion of bytes to types used in the map. `serialize` and `extract`
// works directly with the types used by the map, based on compile time key. `select` and `insert`
// works with the map itself based on runtime information.
template < typename Map >
struct protocol_register_handler
{
        using map_type = Map;
        using key_type = typename map_type::key_type;

        static constexpr std::size_t max_size = map_type::max_value_size;

        using message_type = typename map_type::message_type;

        template < key_type Key >
        static message_type serialize( typename map_type::reg_value_type< Key > val )
        {
                using def =
                    protocol_def< typename map_type::reg_def_type< Key >, PROTOCOL_BIG_ENDIAN >;

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
        static either< typename map_type::reg_value_type< Key >, protocol_error_record >
        extract( const view< const uint8_t* >& msg )
        {
                using def =
                    protocol_def< typename map_type::reg_def_type< Key >, PROTOCOL_BIG_ENDIAN >;

                return def::deserialize( msg ).convert_left( [&]( auto sub_res ) {
                        return sub_res.val;
                } );
        }

        template < typename Buffer >
        static std::optional< protocol_error_record >
        insert( map_type& m, key_type key, Buffer&& buff )
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

}  // namespace emlabcpp
