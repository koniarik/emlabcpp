/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
#pragma once

#include "emlabcpp/algorithm.h"
#include "emlabcpp/view.h"

#include <cstddef>

namespace emlabcpp
{

/// TODO: make this no inline

class cobs_encoder
{
public:
        cobs_encoder( view< std::byte* > target )
          : target( target )
        {
        }

        bool insert( std::byte b )
        {
                if ( b != std::byte{ 0 } ) {
                        count += 1;
                        *p = b;
                } else {
                        *last_p = std::byte{ count };
                        count   = 1;
                        last_p  = p;
                }

                ++p;

                if ( p == target.end() )
                        return false;

                if ( count == 255 ) {
                        *last_p = std::byte{ 255 };
                        count   = 1;
                        last_p  = p++;
                }

                return p != target.end();
        }

        view< std::byte* > commit() &&
        {
                *last_p = std::byte{ count };
                return { target.begin(), p };
        }

private:
        view< std::byte* > target;
        std::byte*         last_p = target.begin();
        std::byte*         p      = std::next( last_p );
        uint8_t            count  = 1;
};

/// Encodes data from source range into target buffer with Consistent Overhead Byte Stuffing (COBS)
/// encoding, returns bool indicating whenever conversion succeeded and subview used for conversion
/// from target buffer. Note that this does not store 0 at the end.
inline std::tuple< bool, view< std::byte* > >
encode_cobs( view< const std::byte* > source, view< std::byte* > target )
{
        cobs_encoder e( target );
        for ( std::byte b : source )
                if ( !e.insert( b ) )
                        return { false, {} };
        return { true, std::move( e ).commit() };
}

struct cobs_decoder
{
        [[nodiscard]] std::optional< std::byte > get( std::byte inpt ) const
        {
                if ( offset == 1 ) {
                        if ( nonzero )
                                return std::nullopt;
                        else
                                return std::byte{ 0 };
                }
                return inpt;
        }

        bool non_value_byte()
        {
                return offset == 1 && nonzero;
        }

        void advance( std::byte inpt )
        {
                if ( offset == 1 ) {
                        nonzero = inpt == std::byte{ 255 };
                        offset  = static_cast< uint8_t >( inpt );
                } else {
                        offset--;
                }
        }

        [[nodiscard]] std::optional< std::byte > iter( std::byte inpt )
        {
                const std::optional< std::byte > b = get( inpt );
                advance( inpt );
                return b;
        }

        bool    nonzero = false;
        uint8_t offset  = 1;

        cobs_decoder() = default;

        cobs_decoder( std::byte b )
          : nonzero( b == std::byte{ 255 } )
          , offset( static_cast< uint8_t >( b ) )
        {
        }
};

/// Decodes data from source range into target buffer with Consistent Overhead Byte Stuffing (COBS)
/// encoding, returns bool indicating whenever conversion succeeded and subview used for conversion
/// from target buffer. Note that this does not expect 0 at the end.
inline std::tuple< bool, view< std::byte* > >
decode_cobs( view< const std::byte* > source, view< std::byte* > target )
{

        std::byte*   target_current = target.begin();
        cobs_decoder dec( source.front() );

        for ( const std::byte b : tail( source ) ) {

                std::optional< std::byte > val = dec.iter( b );

                if ( !val.has_value() )
                        continue;
                *target_current = *val;
                target_current += 1;

                if ( target_current == target.end() )
                        return { false, target };
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
                // note: we do assume that iterator skips places without value
                // NOLINTNEXLINE
                return *dec_.get( *iter_ );
        }

        decode_cobs_iter& operator++()
        {
                dec_.advance( *iter_ );
                ++iter_;
                if ( dec_.non_value_byte() ) {
                        dec_.advance( *iter_ );
                        ++iter_;
                }
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
        cobs_decoder dec_;
        Iter         iter_;
};

template < typename Iter >
view< decode_cobs_iter< Iter >, decode_cobs_iter< Iter > > cobs_decode_view( view< Iter > data )
{
        return {
            ++decode_cobs_iter< Iter >{ data.begin() },
            decode_cobs_iter< Iter >{ find( data, std::byte{ 0 } ) } };
}

}  // namespace emlabcpp
