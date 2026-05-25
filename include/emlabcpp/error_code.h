/// MIT License
///
/// Copyright (c) 2026 Jan Veverak Koniarik
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

#include <concepts>
#include <cstdint>
#include <type_traits>

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
inline undefined_error_category< T > const error_category_v;

template < typename T >
concept error_type = requires { error_category_v< T >; };

struct bool_category : _error_category
{
        [[nodiscard]] constexpr error_value_type cast( bool x ) const
        {
                return x ? 0 : 1;
        }

        [[nodiscard]] char const* message( error_value_type code ) const noexcept override
        {
                return code ? "true" : "false";
        }
};

template <>
inline bool_category const error_category_v< bool > = {};

struct [[nodiscard]] error_code
{
        template < typename T >
        requires( !std::same_as< std::remove_cvref_t< T >, error_code > && error_type< T > )
        error_code( T x )
          : code_( error_category_v< T >.cast( x ) )
          , category_( &error_category_v< T > )
        {
        }

        constexpr error_code( error_code const& ) noexcept            = default;
        constexpr error_code( error_code&& ) noexcept                 = default;
        constexpr error_code& operator=( error_code const& ) noexcept = default;
        constexpr error_code& operator=( error_code&& ) noexcept      = default;

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
                return code_ == 0;
        }

        [[nodiscard]] constexpr error_value_type value() const noexcept
        {
                return code_;
        }

        [[nodiscard]] constexpr bool operator==( error_code const& ) const noexcept = default;

        template < typename T >
        requires( !std::same_as< std::remove_cvref_t< T >, error_code > && error_type< T > )
        [[nodiscard]] constexpr bool operator==( T x ) const noexcept
        {
                return *this == error_code( x );
        }

private:
        error_value_type       code_;
        _error_category const* category_;
};

}  // namespace emlabcpp
