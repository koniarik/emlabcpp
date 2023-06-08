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

template < typename Iter >
class decode_cobs_iter;

}  // namespace emlabcpp

template < typename Iter >
struct std::iterator_traits< emlabcpp::decode_cobs_iter< Iter > >
{
        using value_type        = std::byte;
        using difference_type   = std::ptrdiff_t;
        using pointer           = value_type*;
        using const_pointer     = const value_type*;
        using reference         = value_type&;
        using iterator_category = std::input_iterator_tag;
};

namespace emlabcpp
{

template < typename Iter >
class decode_cobs_iter : public generic_iterator< decode_cobs_iter< Iter > >
{
public:
        decode_cobs_iter( Iter iter )
          : iter_( iter )
        {
        }

        std::byte operator*() const
        {
                if ( offset_ == 0 ) {
                        return std::byte{ 0 };
                }
                return *iter_;
        }

        decode_cobs_iter& operator++()
        {
                if ( offset_ == 0 ) {
                        offset_ = static_cast< uint8_t >( *iter_ ) - 1;
                } else {
                        offset_--;
                }
                ++iter_;
                return *this;
        }

        decode_cobs_iter operator++( int )
        {
                decode_cobs_iter res{ *this };
                ( *this )++;
                return res;
        }

        bool operator==( const decode_cobs_iter& other ) const
        {
                return iter_ == other.iter_;
        }

private:
        uint8_t offset_ = 0;
        Iter    iter_;
};

template < typename Iter >
view< decode_cobs_iter< Iter >, decode_cobs_iter< Iter > > cobs_decode_view( view< Iter > data )
{
        return {
            ++decode_cobs_iter< Iter >{ data.begin() },
            decode_cobs_iter< Iter >{ find( data, std::byte{ 0 } ) } };
}

}  // namespace emlabcpp
