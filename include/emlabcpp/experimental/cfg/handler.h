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

#include "./load.h"
#include "./store.h"

namespace emlabcpp::cfg
{

template < typename Payload, typename Field, std::endian Endianess >
struct handler
{
        using chcksum_conv = protocol::converter_for< checksum, Endianess >;

        template < typename ChecksumFunction >
        static std::tuple< bool, std::span< std::byte > > store(
            std::span< std::byte > target_buffer,
            Payload const&         pl,
            view< Field const* >   fields,
            ChecksumFunction&&     chcksm_f )
        {
                return store(
                    target_buffer,
                    pl,
                    fields.size(),
                    [&]( std::size_t i ) {
                            return fields[i];
                    },
                    chcksm_f );
        }

        template < typename FieldFunction, typename ChecksumFunction >
        static std::tuple< bool, std::span< std::byte > > store(
            std::span< std::byte > target_buffer,
            Payload const&         pl,
            std::size_t            field_count,
            FieldFunction&&        field_f,
            ChecksumFunction&&     chcksm_f )
        {
                bool                   success;
                std::span< std::byte > buffer = target_buffer;

                std::tie( success, buffer ) = store_impl< Endianess >(
                    buffer,
                    header{
                        .field_count = static_cast< uint32_t >( field_count ),
                    },
                    chcksm_f );
                if ( !success )
                        return { false, {} };
                std::tie( success, buffer ) = store_impl< Endianess >( buffer, pl, chcksm_f );
                if ( !success )
                        return { false, {} };

                for ( std::size_t const i : range( field_count ) ) {
                        Field const fp = field_f( i );
                        std::tie( success, buffer ) =
                            store_impl< Endianess >( buffer, fp, chcksm_f );
                        if ( !success )
                                return { false, {} };
                }

                return { success, target_buffer.subspan( 0, buffer.size() ) };
        }

        template < typename PayloadFunction, typename FieldFunction, typename ChecksumFunction >
        static load_result load(
            std::span< std::byte > source,
            PayloadFunction&&      pl_f,
            FieldFunction&&        field_f,
            ChecksumFunction&&     chcksm_f )
        {
                std::span< std::byte > buffer = source;
                bool                   success;

                header head;
                std::tie( success, head, buffer ) =
                    load_impl< header, Endianess >( buffer, chcksm_f );
                if ( !success )
                        return load_result::DESERIALIZATION_ERROR;

                Payload pl;
                std::tie( success, pl, buffer ) =
                    load_impl< Payload, Endianess >( buffer, chcksm_f );
                if ( !success )
                        return load_result::DESERIALIZATION_ERROR;

                if ( !pl_f( std::as_const( pl ) ) )
                        return load_result::PAYLOAD_REFUSED;

                for ( std::size_t const i : range( head.field_count ) ) {
                        std::ignore = i;
                        Field f;
                        std::tie( success, f, buffer ) =
                            load_impl< Field, Endianess >( buffer, chcksm_f );
                        if ( !success )
                                return load_result::DESERIALIZATION_ERROR;
                        field_f( std::as_const( f ) );
                }
                return load_result::SUCCESS;
        }
};

}  // namespace emlabcpp::cfg
