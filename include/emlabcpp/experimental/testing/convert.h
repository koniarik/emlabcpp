
#pragma once

namespace emlabcpp::testing
{

template < typename T >
struct testing_value_converter;

template < typename T >
requires( alternative_of< T, testing_value > ) struct testing_value_converter< T >
{
        static std::optional< T > from_value( const testing_value& var )
        {
                const T* val_ptr = std::get_if< T >( &var );
                if ( val_ptr ) {
                        return *val_ptr;
                }
                return std::nullopt;
        }

        static testing_value to_value( const T& item )
        {
                return { item };
        }
};

template < typename T >
requires(
    !std::same_as< T, int64_t > && std::is_integral_v< T > ) struct testing_value_converter< T >
{
        static std::optional< T > from_value( const testing_value& var )
        {
                if ( std::holds_alternative< float >( var ) ) {
                        float v = *std::get_if< float >( &var );
                        if ( v == 0.f ) {
                                return T{ 0 };
                        }
                }
                std::optional< int64_t > opt_val =
                    testing_value_converter< int64_t >::from_value( var );
                if ( !opt_val ) {
                        return std::nullopt;
                }
                return static_cast< T >( *opt_val );
        }

        static testing_value to_value( const T& item )
        {
                return { static_cast< int64_t >( item ) };
        }
};

template <>
struct testing_value_converter< std::string_view >
{
        static testing_value to_value( const std::string_view& item )
        {
                return testing_string_to_buffer( item );
        }
};

}  // namespace emlabcpp::testing
