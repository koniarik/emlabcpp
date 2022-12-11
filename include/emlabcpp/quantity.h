// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//
//  Copyright © 2022 Jan Veverak Koniarik
//  This file is part of project: emlabcpp
//
#include "emlabcpp/concepts.h"

#include <cmath>
#include <functional>
#include <limits>
#include <ratio>
#include <string>
#include <type_traits>

#ifdef EMLABCPP_USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

#pragma once

namespace emlabcpp
{

/// Class representing generic quantity.
///
/// Quantities are types which simply overlay basic numeric type (ValueType) and give you abillity
/// to create custom types via CRTP. The C++ type system prevents you from passing values of
/// quantites of different implementation type.
///
/// The overlay implements:
/// 	+=,-=
/// 	+,-
/// 	==, !=
/// 	<,>,>=,<=
/// 	abs, max, min
/// Quantity can be multiplied or divided by it's ValueType - /,*,/=,*=
/// Additionally, we support these operations over quantity:
/// 	cos, sin
///
/// Credits should go to https://github.com/joboccara/NamedType as I inspired by project by this
/// blogger!
///
template < typename Derived, typename ValueType = float >
class quantity
{
        ValueType value_;

        [[nodiscard]] Derived& impl()
        {
                return static_cast< Derived& >( *this );
        }
        [[nodiscard]] Derived const& impl() const
        {
                return static_cast< Derived const& >( *this );
        }

public:
        using value_type = ValueType;

        constexpr quantity() noexcept
          : value_( 0 )
        {
        }

        /// Default constructor used to create a physical quantity from value
        template < typename Value >
        constexpr explicit quantity( Value val ) noexcept
          : value_( static_cast< ValueType >( val ) )
        {
        }

        /// Const reference to the internal value
        constexpr ValueType operator*() const noexcept
        {
                return value_;
        }

        /// Add other quantity of same Derived and value_type
        constexpr Derived& operator+=( const quantity other ) noexcept
        {
                value_ += *other;
                return impl();
        }

        /// Subtract other quantity of same Derived and value_type
        constexpr Derived& operator-=( const quantity other ) noexcept
        {
                value_ -= *other;
                return impl();
        }

        /// Divides quantity by it's value type
        constexpr Derived& operator/=( const arithmetic auto val ) noexcept
        {
                value_ /= val;
                return impl();
        }

        /// Multiplies quantity by it's value type
        constexpr Derived& operator*=( const arithmetic auto val ) noexcept
        {
                value_ *= val;
                return impl();
        }

        /// Provides explicit conversion of internal value to type U
        template < typename U >
        constexpr explicit operator U() const noexcept
        {
                return U( value_ );
        }

        static std::string get_unit()
        {
                return "";
        }

        friend constexpr auto operator<=>( const quantity&, const quantity& ) = default;
};

namespace detail
{
        template < typename Derived, typename ValueType >
        constexpr bool quantity_derived_test( const quantity< Derived, ValueType >& )
        {
                return true;
        }
}  // namespace detail

/// Concept satisfies any type `T` that inherits from any form of `quantity<U>`.
template < typename T >
concept quantity_derived = requires( T val )
{
        detail::quantity_derived_test( val );
};

/// Class represents a quantity that uses `tags` to distinguish quantities instead of inheritance.
///
/// Usefull because it is simpler to write template alias such as this:
/// `using apple_count = tagget_quantity< struct apple_count_tag, uint32_t >;`
///
/// Writing full class would take more effort, but class has the benefit of better error messages.
template < typename Tag, typename ValueType = float >
class tagged_quantity : public quantity< tagged_quantity< Tag, ValueType >, ValueType >
{
public:
        using tag = Tag;
        using quantity< tagged_quantity< Tag, ValueType >, ValueType >::quantity;
};

/// Sum of quantities with same Derived and value_type
template < typename Derived, typename ValueType >
constexpr Derived
operator+( quantity< Derived, ValueType > lhs, const quantity< Derived, ValueType > rhs )
{
        return lhs += rhs;
}

/// Subtraction of quantities with same Derived and value_type
template < typename Derived, typename ValueType >
constexpr Derived
operator-( quantity< Derived, ValueType > lhs, const quantity< Derived, ValueType > rhs )
{
        return lhs -= rhs;
}

/// Provides negation of the quantity
template < typename Derived, typename ValueType >
constexpr Derived operator-( const quantity< Derived, ValueType > val )
{
        return Derived{ -*val };
}

/// Provides abillity to compare quantity with non-quantity arithmetic value.
template < typename Derived, typename ValueType, arithmetic_like RhValueType >
constexpr bool operator<( const quantity< Derived, ValueType > lhs, const RhValueType rhs )
{
        return *lhs < rhs;
}

/// Provides abillity to compare quantity with non-quantity arithmetic value.
template < typename Derived, typename ValueType, arithmetic_like LhValueType >
constexpr bool operator<( const LhValueType lhs, const quantity< Derived, ValueType > rhs )
{
        return lhs < *rhs;
}

/// Multiplication of quantity by it's value_type
template < typename Derived, typename ValueType >
constexpr Derived operator*( quantity< Derived, ValueType > q, const arithmetic auto val )
{
        return q *= val;
}

/// Division of quantity by it's value_type
template < typename Derived, typename ValueType >
constexpr Derived operator/( quantity< Derived, ValueType > q, const arithmetic auto val )
{
        return q /= val;
}

/// Quantity with absolute value of internal value
template < typename Derived, typename ValueType >
constexpr Derived abs( const quantity< Derived, ValueType > q )
{
        return Derived( std::abs( *q ) );
}

/// Returns cosinus of the quantity as scalar
template < typename Derived, typename ValueType >
constexpr auto cos( const quantity< Derived, ValueType > u )
{
        return std::cos( *u );
}

/// Returns sinus of the quantity as scalar
template < typename Derived, typename ValueType >
constexpr auto sin( const quantity< Derived, ValueType > u )
{
        return std::sin( *u );
}

/// Quantity with maximum value of one of the quantities
template < typename Derived, typename ValueType >
constexpr Derived
max( const quantity< Derived, ValueType > lh, const quantity< Derived, ValueType > rh )
{
        return Derived( std::max( *lh, *rh ) );
}

/// Quantity with minimum value of one of the quantities
template < typename Derived, typename ValueType >
constexpr Derived
min( const quantity< Derived, ValueType > lh, const quantity< Derived, ValueType > rh )
{
        return Derived( std::min( *lh, *rh ) );
}

//---------------------------------------------------------------------------

/// Multiplication of value_type by quantity returns quantity
template < typename Derived, typename ValueType >
constexpr Derived operator*( const ValueType val, const quantity< Derived, ValueType > q )
{
        return q * val;
}
/// Division of value_type by quantity returns quantity
template < typename Derived, typename ValueType >
constexpr ValueType operator/( const ValueType val, const quantity< Derived, ValueType > q )
{
        return val / *q;
}

template < ostreamlike Stream, typename T, typename ValueType >
auto& operator<<( Stream& os, const quantity< T, ValueType >& q )
{
        return os << *q << T::get_unit();
}

}  // namespace emlabcpp

/// The quantity has defined partital specialization of std::numeric_limits,
/// works as std::numeric_limits<ValueType>;
template < emlabcpp::quantity_derived T >
struct std::numeric_limits< T >
{
        using value_type = typename T::value_type;

        constexpr static T lowest()
        {
                return T{ std::numeric_limits< value_type >::lowest() };
        }
        constexpr static T min()
        {
                return T{ std::numeric_limits< value_type >::min() };
        }
        constexpr static T max()
        {
                return T{ std::numeric_limits< value_type >::max() };
        }
};

/// Hash of quantity is hash of it's value and Derived::get_unit() xored.
template < emlabcpp::quantity_derived T >
struct std::hash< T >
{
        std::size_t operator()( T q ) const
        {
                /// TODO: this should be rewritten
                /// 'reverse' the prefix+unit info in bits and than xor it with number
                const std::string unit = T::get_unit();
                return std::hash< typename T::value_type >()( *q ) ^
                       std::hash< std::string >()( unit );
        }
};

#ifdef EMLABCPP_USE_NLOHMANN_JSON

template < emlabcpp::quantity_derived T >
struct nlohmann::adl_serializer< T >
{
        static void to_json( nlohmann::json& j, const T q )
        {
                if ( T::get_unit().empty() ) {
                        j = *q;
                } else {
                        j[T::get_unit()] = *q;
                }
        }

        static T from_json( const nlohmann::json& j )
        {
                using value_type = typename T::value_type;

                if ( T::get_unit().empty() ) {
                        return T{ j.get< value_type >() };
                }
                return T{ j[T::get_unit()].template get< value_type >() };
        }
};

#endif
