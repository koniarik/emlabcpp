#pragma once

#include <cstdint>

namespace emlabcpp
{

using error_value_type = uint32_t;

struct _error_category
{
        [[nodiscard]]
        virtual char const* message( error_value_type ) const noexcept = 0;
        virtual ~_error_category()                                     = default;
};

template < typename T >
struct error_category : _error_category
{
        [[nodiscard]] constexpr error_value_type cast( T& x ) const
        {
                return static_cast< error_value_type >( x );
        }
};

template < typename T >
struct undefined_error_category : error_category< T >
{
        static_assert(
            sizeof( T ) == 0,
            "Undefined error category used. Specialize error_category for this type." );
};

template < typename T >
extern undefined_error_category< T > const error_category_v;

template < typename T >
concept error_type = requires { error_category_v< T >; };

struct bool_category : _error_category
{
        [[nodiscard]] constexpr error_value_type cast( bool x ) const
        {
                return x ? 1 : 0;
        }

        [[nodiscard]] char const* message( error_value_type code ) const noexcept override
        {
                return code ? "true" : "false";
        }
};

template <>
inline constexpr bool_category error_category_v< bool > = {};

struct [[nodiscard]] error_code
{
        template < error_type T >
        constexpr error_code( T x )
          : code_( error_category_v< T >.cast( x ) )
          , category_( &error_category_v< T > )
        {
        }

        [[nodiscard]] constexpr char const* message() const noexcept
        {
                return category_->message( code_ );
        }

        [[nodiscard]] constexpr _error_category const& category() const noexcept
        {
                return *category_;
        }

        [[nodiscard]] constexpr operator bool() const noexcept
        {
                return code_ != 0;
        }

        [[nodiscard]] constexpr error_value_type value() const noexcept
        {
                return code_;
        }

        [[nodiscard]] constexpr bool operator==( error_code const& ) const noexcept = default;

        template < error_type T >
        [[nodiscard]] constexpr bool operator==( T x ) const noexcept
        {
                return *this == error_code( x );
        }

private:
        error_value_type       code_;
        _error_category const* category_;
};

}  // namespace emlabcpp