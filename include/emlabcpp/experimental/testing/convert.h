#include "emlabcpp/experimental/testing/base.h"

#pragma once

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
                if ( val_ptr ) {
                        return *val_ptr;
                }
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

template < typename T >
requires( alternative_of< T, value_type > ) struct value_type_converter< T >
  : value_type_converter_getter< T >
{
};

template < typename T >
requires( !std::same_as< T, int64_t > && std::is_integral_v< T > ) struct value_type_converter< T >
{
        static std::optional< T > from_value( const value_type& var )
        {
                if ( std::holds_alternative< float >( var ) ) {
                        const auto v = *std::get_if< float >( &var );
                        if ( v == 0.f ) {
                                return T{ 0 };
                        }
                }
                std::optional< int64_t > opt_val =
                    value_type_converter< int64_t >::from_value( var );
                if ( !opt_val ) {
                        return std::nullopt;
                }
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
                return string_to_buffer( item );
        }
};

}  // namespace emlabcpp::testing
