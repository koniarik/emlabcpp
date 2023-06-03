#include "emlabcpp/algorithm.h"
#include "emlabcpp/view.h"

#include <cstddef>

#pragma once

namespace emlabcpp
{

/// TODO: make this no inline

/// Encodes data from source range into target buffer with Consistent Overhead Byte Stuffing (COBS)
/// encoding, returns bool indicating whenever conversion succeeded and subview used for conversion
/// from target buffer. Note that this does not store 0 at the end.
inline std::tuple< bool, view< std::byte* > >
encode_cobs( view< std::byte* > source, view< std::byte* > target )
{
        std::byte* last_tok       = target.begin();
        std::byte* target_current = last_tok;
        target_current++;
        uint8_t count = 1;
        for ( const std::byte b : source ) {
                if ( b == std::byte{ 0 } ) {
                        *last_tok = std::byte{ count };
                        count     = 1;
                        last_tok  = target_current;
                } else {
                        count += 1;
                        *target_current = b;
                }

                target_current += 1;

                if ( target_current == target.end() ) {
                        return { false, target };
                }
        }
        *last_tok = std::byte{ count };
        return { true, { target.begin(), target_current } };
}

/// Decodes data from source range into target buffer with Consistent Overhead Byte Stuffing (COBS)
/// encoding, returns bool indicating whenever conversion succeeded and subview used for conversion
/// from target buffer. Note that this does not expect 0 at the end.
inline std::tuple< bool, view< std::byte* > >
decode_cobs( view< std::byte* > source, view< std::byte* > target )
{

        std::byte* target_current = target.begin();
        uint8_t    count          = static_cast< uint8_t >( *source.begin() );

        for ( const std::byte b : tail( source ) ) {
                count -= 1;
                if ( count == 0 ) {
                        count           = static_cast< uint8_t >( b );
                        *target_current = std::byte{ 0 };
                } else {
                        *target_current = b;
                }
                target_current += 1;
                if ( target_current == target.end() ) {
                        return { false, target };
                }
        }
        return { true, { target.begin(), target_current } };
}

}  // namespace emlabcpp
