#include "emlabcpp/concepts.h"

#include <optional>
#include <type_traits>

#pragma once

namespace emlabcpp
{

/// Bounded type represents a overlay over type T which is constrained between MinVal and MaxVal as
/// compile time constants. / The API is deisgned in a way that you can't create the type out of
/// theb ounds.
///
/// This is beneficial in design of an API.
/// In case of using the T directly and asserting that it is withing corrent range, the API has to
/// check at runtime whenever that is true and return error in case it is out of range. / With
/// bounded type the API does not have to do that, as the type can't be passed unless it is within
/// corret range.
///
/// It does not remove the bounds check, it just moves it away from within API to the user, which
/// has to deal with creating correct bounded type.
template < typename T, T MinVal, T MaxVal >
class bounded
{
        static_assert( MinVal <= MaxVal );
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

        template < typename U, U OtherMin, U OtherMax >
        requires(
            std::is_integral_v< U >&& std::is_integral_v< T >&& min_val <= OtherMin &&
            OtherMax <= max_val ) constexpr bounded( bounded< U, OtherMin, OtherMax > other )
          : val_( static_cast< T >( *other ) )
        {
        }

        /// Static method that creates an instance of bounded with value provided at compile time.
        /// This value is checked at compile time.
        template < T Val >
        static constexpr bounded get()
        {
                static_assert( min_val <= Val && Val <= max_val );
                return bounded{ Val };
        }

        /// Static method optinally returns bounded value, only in case the input value is withing
        /// allowed range.
        template < typename U >
        static std::optional< bounded< T, min_val, max_val > > make( U val )
        {
                if ( static_cast< T >( val ) < min_val ) {
                        return {};
                }
                if ( static_cast< T >( val ) > max_val ) {
                        return {};
                }
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

        friend constexpr auto operator<=>( const bounded&, const bounded& ) = default;

        template < typename U >
        friend constexpr auto operator<=>( const bounded& b, const U& val )
        {
                return *b <=> val;
        }
};

/// Simple type alias for bounded index constants.
template < std::size_t N >
constexpr auto bounded_constant = bounded< std::size_t, N, N >{};

/// Sum of two bounded types of same base type is bounded within appropiate ranges.
template < typename T, T FromLh, T ToLh, T FromRh, T ToRh >
constexpr bounded< T, FromLh + FromRh, ToLh + ToRh >
operator+( const bounded< T, FromLh, ToLh >& lh, const bounded< T, FromRh, ToRh >& rh )
{
        return *bounded< T, FromLh + FromRh, ToLh + ToRh >::make( *lh + *rh );
}

namespace detail
{
        template < typename T, T MinVal, T MaxVal >
        constexpr bool bounded_derived_test( const bounded< T, MinVal, MaxVal >& )
        {
                return true;
        }
}  /// namespace detail

/// Concept that matchestype deriving from bounded
template < typename T >
concept bounded_derived = requires( T val )
{
        detail::bounded_derived_test( val );
};

template < ostreamlike Stream, typename T, T MinVal, T MaxVal >
inline auto& operator<<( Stream& os, const bounded< T, MinVal, MaxVal >& b )
{
        return os << *b;
}

}  /// namespace emlabcpp
