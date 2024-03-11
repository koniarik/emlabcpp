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

#include "emlabcpp/experimental/string_buffer.h"
#include "emlabcpp/experimental/testing/base.h"

namespace emlabcpp::testing
{

template < typename T >
struct value_type_converter;

template < typename T >
struct value_type_converter_getter
{
        static std::optional< T > from_value( const value_type& var )
        {
                const T* val_ptr = std::get_if< T >( &var );
                if ( val_ptr )
                        return *val_ptr;
                return std::nullopt;
        }

        static value_type to_value( const T& item )
        {
                return { item };
        }
};

template <>
struct value_type_converter< int64_t > : value_type_converter_getter< int64_t >
{
};

template < std::size_t N >
requires( N <= string_buffer::capacity )
struct value_type_converter< emlabcpp::string_buffer< N > >
{
        static std::optional< emlabcpp::string_buffer< N > > from_value( const value_type& var )
        {
                const auto* val_ptr = std::get_if< string_buffer >( &var );
                if ( val_ptr )
                        return *val_ptr;
                return std::nullopt;
        }

        static value_type to_value( const emlabcpp::string_buffer< N >& item )
        {
                return { string_buffer{ item } };
        }
};

template < typename T >
requires( alternative_of< T, value_type > )
struct value_type_converter< T > : value_type_converter_getter< T >
{
};

template < typename T >
requires( !std::same_as< T, int64_t > && std::is_integral_v< T > && !std::same_as< T, bool > )
struct value_type_converter< T >
{
        static std::optional< T > from_value( const value_type& var )
        {
                if ( std::holds_alternative< float >( var ) ) {
                        const auto v = *std::get_if< float >( &var );
                        if ( v == 0.f )
                                return T{ 0 };
                }
                std::optional< int64_t > opt_val =
                    value_type_converter< int64_t >::from_value( var );
                if ( !opt_val )
                        return std::nullopt;
                return static_cast< T >( *opt_val );
        }

        static value_type to_value( const T& item )
        {
                return { static_cast< int64_t >( item ) };
        }
};

template <>
struct value_type_converter< std::string_view >
{
        static value_type to_value( const std::string_view& item )
        {
                return string_buffer( item );
        }
};

template <>
struct value_type_converter< const char* >
{
        static value_type to_value( const char* item )
        {
                return string_buffer( item );
        }
};

template < typename Rep, typename Ratio >
struct value_type_converter< std::chrono::duration< Rep, Ratio > >
{
        static value_type to_value( const std::chrono::duration< Rep, Ratio >& val )
        {
                return value_type_converter< Rep >::to_value( val.count() );
        }

        static std::optional< std::chrono::duration< Rep, Ratio > >
        from_value( const value_type& var )
        {
                std::optional< Rep > opt_raw = value_type_converter< Rep >::from_value( var );
                if ( !opt_raw.has_value() )
                        return std::nullopt;
                return std::chrono::duration< Rep, Ratio >{ *opt_raw };
        }
};

template < typename T >
concept value_type_convertible = requires( const T& item, const value_type& val ) {
        {
                value_type_converter< T >::to_value( item )
        } -> std::convertible_to< value_type >;
        {
                value_type_converter< T >::from_value( val )
        } -> std::convertible_to< std::optional< T > >;
};

}  // namespace emlabcpp::testing
