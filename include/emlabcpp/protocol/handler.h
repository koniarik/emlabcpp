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

#include "emlabcpp/either.h"
#include "emlabcpp/protocol/converter.h"
#include "emlabcpp/types.h"

#pragma once

namespace emlabcpp::protocol
{

/// handler< T > should be used to execute actual serialization and deserealization of
/// protocol definition. It provides serialize/extract methods that should be used by the user.
//
/// You may want to have this class (With the include) to be present in separate .cpp file, as the
/// compile time can be quite heavy.
template < convertible T >
struct handler
{
        using def                             = converter_for< T, std::endian::big >;
        static constexpr std::size_t max_size = def::max_size;
        using value_type                      = typename def::value_type;
        using message_type                    = message< max_size >;

        static message_type serialize( const value_type& val )
        {
                message_type res{ max_size };

                const bounded used =
                    def::serialize_at( std::span< std::byte, max_size >{ res }, val );
                EMLABCPP_ASSERT( *used <= max_size );
                res.resize( *used );
                return res;
        };

        static either< value_type, error_record > extract( const view< const std::byte* >& msg )
        {
                value_type        val;
                conversion_result res = def::deserialize( msg, val );
                if ( res.has_error() ) {
                        const mark* const mark = res.get_error();
                        EMLABCPP_ERROR_LOG(
                            "Failed to extract protocol def ",
                            pretty_type_name< T >(),
                            " from message ",
                            message_type( msg ),
                            ", error is: ",
                            *mark,
                            " with ",
                            res.used,
                            " bytes" );
                        return error_record{ *mark, res.used };
                }
                return val;
        }
};

}  // namespace emlabcpp::protocol
