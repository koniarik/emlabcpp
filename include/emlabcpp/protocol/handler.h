#include "emlabcpp/either.h"
#include "emlabcpp/protocol/def.h"

#pragma once

namespace emlabcpp
{

// protocol_handler< T > should be used to execute actual serialization and deserealization of
// protocol definition. It provides serialize/extract methods that should be used by the user.
//
// You may want to have this class (With the include) to be present in separate .cpp file, as the
// compile time can be quite heavy.
template < protocol_declarable T >
struct protocol_handler
{
        using def          = protocol_def< T, PROTOCOL_BIG_ENDIAN >;
        using value_type   = typename def::value_type;
        using message_type = protocol_message< def::max_size >;

        static message_type serialize( value_type val )
        {
                std::array< uint8_t, def::max_size > buffer{};

                bounded used = def::serialize_at( buffer, val );
                EMLABCPP_ASSERT( *used <= def::max_size );
                return *message_type::make( view_n( buffer.begin(), *used ) );
        };

        static either< value_type, protocol_error_record >
        extract( const view< const uint8_t* >& msg )
        {
                auto opt_view = bounded_view< const uint8_t*, typename def::size_type >::make(
                    view_n( msg.begin(), min( def::max_size, msg.size() ) ) );
                if ( !opt_view ) {
                        return protocol_error_record{ SIZE_ERR, 0 };
                }
                auto [used, res] = def::deserialize( *opt_view );
                if ( std::holds_alternative< const protocol_mark* >( res ) ) {
                        return protocol_error_record{ *std::get< 1 >( res ), used };
                }
                return std::get< 0 >( res );
        }
};

}  // namespace emlabcpp
