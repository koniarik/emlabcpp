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
///

#pragma once

#include "./concepts.h"

#include <optional>
#include <type_traits>

namespace emlabcpp
{

/// The `bounded` class represents a wrapper over type `T` constrained between `MinVal` and `MaxVal`
/// as compile-time constants. This ensures that values of this type are always within the specified
/// range, providing compile-time guarantees and eliminating the need for runtime checks within
/// APIs.
///
/// This design shifts the responsibility of bounds checking from the API to the user, ensuring that
/// only valid values are passed to the API. While bounds checks are still necessary, they are moved
/// to the point of creation of the `bounded` type.
template < typename T, T MinVal, T MaxVal >
class bounded
{
        static_assert( MinVal <= MaxVal, "MinVal must be less than or equal to MaxVal" );
        T val_;

        constexpr explicit bounded( T val )
          : val_( val )
        {
        }

public:
        static constexpr T    min_val            = MinVal;
        static constexpr T    max_val            = MaxVal;
        static constexpr T    interval_range     = static_cast< T >( 1 ) + max_val - min_val;
        static constexpr bool has_single_element = MinVal == MaxVal;

        /// Constructor that allows conversion from another `bounded` type with compatible bounds.
        template < typename U, U OtherMin, U OtherMax >
        requires(
            std::is_integral_v< U > && std::is_integral_v< T > && min_val <= OtherMin &&
            OtherMax <= max_val )
        constexpr bounded( bounded< U, OtherMin, OtherMax > other )
          : val_( static_cast< T >( *other ) )
        {
        }

        /// Creates an instance of `bounded` with a value provided at compile time.
        /// The value is checked at compile time to ensure it is within the allowed range.
        template < T Val >
        static constexpr bounded get()
        {
                static_assert( min_val <= Val && Val <= max_val, "Value out of bounds" );
                return bounded{ Val };
        }

        /// Creates an optional `bounded` value if the input value is within the allowed range.
        /// Returns `std::nullopt` if the value is out of bounds.
        template < typename U >
        static std::optional< bounded< T, min_val, max_val > > make( U val )
        {
                if ( static_cast< T >( val ) < min_val )
                        return {};
                if ( static_cast< T >( val ) > max_val )
                        return {};
                return bounded{ static_cast< T >( val ) };
        }

        static bounded< T, min_val, max_val > min()
        {
                return bounded{ min_val };
        }

        static bounded< T, min_val, max_val > max()
        {
                return bounded{ max_val };
        }

        constexpr bounded()
          : val_( min_val )
        {
        }

        constexpr T operator*() const
        {
                return val_;
        }

        explicit operator T() const
        {
                return val_;
        }

        /// Rotation to the right increases the internal value by step modulo the range it is in.
        void rotate_right( T step )
        {
                val_ = min_val + ( interval_range + ( val_ + step - min_val ) % interval_range ) %
                                     interval_range;
        }

        /// Rotation to the left decreases the internal value by step modulo the range it is in.
        void rotate_left( T step )
        {
                val_ = min_val + ( interval_range + ( val_ - step - min_val ) % interval_range ) %
                                     interval_range;
        }

        friend constexpr auto operator<=>( bounded const&, bounded const& ) = default;

        template < typename U >
        friend constexpr auto operator<=>( bounded const& b, U const& val )
        {
                return *b <=> val;
        }

        /// Sum of two bounded types of same base type is bounded within appropiate ranges.
        template < T FromOther, T ToOther >
        constexpr bounded< T, MinVal + FromOther, MaxVal + ToOther >
        operator+( bounded< T, FromOther, ToOther > const& other ) const
        {
                return bounded< T, MinVal + FromOther, MaxVal + ToOther >( val_ + *other );
        }

        template < typename U, U FromOther, U ToOther >
        friend class bounded;
};

/// Simple type alias for bounded index constants.
template < std::size_t N >
constexpr auto bounded_constant = bounded< std::size_t, N, N >{};

namespace detail
{
        template < typename T, T MinVal, T MaxVal >
        constexpr bool bounded_derived_test( bounded< T, MinVal, MaxVal > const& )
        {
                return true;
        }
}  // namespace detail

/// Concept that matchestype deriving from bounded
template < typename T >
concept bounded_derived = requires( T val ) { detail::bounded_derived_test( val ); };

#ifdef EMLABCPP_USE_OSTREAM
template < typename T, T MinVal, T MaxVal >
std::ostream& operator<<( std::ostream& os, bounded< T, MinVal, MaxVal > const& b )
{
        return os << *b;
}
#endif

}  // namespace emlabcpp
