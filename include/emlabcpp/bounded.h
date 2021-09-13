#include <optional>
#include <type_traits>

#ifdef EMLABCPP_USE_STREAMS
#include <ostream>
#endif

#pragma once

namespace emlabcpp
{

template < typename T, T MinVal, T MaxVal >
class bounded
{
        T val_;

        constexpr explicit bounded( T val )
          : val_( val )
        {
        }

public:
        static constexpr T    min_val            = MinVal;
        static constexpr T    max_val            = MaxVal;
        static constexpr bool has_single_element = MinVal == MaxVal;

        template < typename U, U OtherMin, U OtherMax >
        requires(
            std::is_integral_v< U >&& std::is_integral_v< T >&& min_val <= OtherMin &&
            OtherMax <= max_val ) constexpr bounded( bounded< U, OtherMin, OtherMax > other )
          : val_( static_cast< T >( *other ) )
        {
        }

        template < T Val >
        static constexpr bounded get()
        {
                static_assert( min_val <= Val && Val <= max_val );
                return bounded{ Val };
        }

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

        T operator*() const
        {
                return val_;
        }

        explicit operator T() const
        {
                return val_;
        }

        friend constexpr auto operator<=>( const bounded&, const bounded& ) = default;

        template < std::totally_ordered_with< T > U >
        friend constexpr auto operator<=>( const bounded& b, const U& val )
        {
                return *b <=> val;
        }
};

template < std::size_t N >
constexpr auto bounded_constant = bounded< std::size_t, N, N >{};

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
}  // namespace detail

template < typename T >
concept bounded_derived = requires( T val )
{
        detail::bounded_derived_test( val );
};

#ifdef EMLABCPP_USE_STREAMS
template < typename T, T MinVal, T MaxVal >
std::ostream& operator<<( std::ostream& os, const bounded< T, MinVal, MaxVal >& b )
{
        if constexpr ( std::is_same_v< T, uint8_t > ) {
                return os << int( *b );
        } else {
                return os << *b;
        }
}
#endif

}  // namespace emlabcpp
