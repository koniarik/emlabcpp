#include <cmath>
#include <functional>
#include <limits>
#include <ratio>
#include <type_traits>

#pragma once

namespace emlabcpp {

/** Class representing generic quantity.
 *
 * Quantities are types which simply overlay basic numeric type (ValueType) and give you abillity to
 * create custom types via CRTP. The C++ type system prevents you from passing values of quantites
 * of different implementation type.
 *
 * The overlay implements:
 * 	+=,-=
 * 	+,-
 * 	==, !=
 * 	<,>,>=,<=
 * 	abs, max, min
 * Quantity can be multiplied or divided by it's ValueType - /,*,/=,*=
 * Additionally, we support these operations over quantity:
 * 	cos, sin
 *
 * Credits should go to https://github.com/joboccara/NamedType as I inspired by project by this
 * blogger!
 */
template <typename T, typename ValueType = float>
class quantity {
        ValueType value_;

        [[nodiscard]] T &      impl() { return static_cast<T &>(*this); }
        [[nodiscard]] T const &impl() const { return static_cast<T const &>(*this); }

      public:
        using value_type = ValueType;

        constexpr quantity() noexcept : value_(0) {}

        // Default constructor used to create a physical quantity from value
        constexpr explicit quantity(ValueType val) noexcept : value_(val) {}

        // Const reference to the internal value
        constexpr ValueType operator*() const noexcept { return value_; }

        // Add other quantity of same T and value_type
        constexpr T &operator+=(const quantity other) noexcept {
                value_ += *other;
                return impl();
        }

        // Subtract other quantity of same T and value_type
        constexpr T &operator-=(const quantity other) noexcept {
                value_ -= *other;
                return impl();
        }

        // Divides quantity by it's value type
        constexpr T &operator/=(const ValueType val) noexcept {
                value_ /= val;
                return impl();
        }

        // Multiplies quantity by it's value type
        constexpr T &operator*=(const ValueType val) noexcept {
                value_ *= val;
                return impl();
        }

        // Provides explicit conversion of internal value to type U
        template <typename U>
        constexpr explicit operator U() const noexcept {
                return U(value_);
        }
};

// Sum of quantities with same T and value_type
template <typename T, typename ValueType>
constexpr T operator+(quantity<T, ValueType> lhs, const quantity<T, ValueType> rhs) {
        return lhs += rhs;
}

// Subtraction of quantities with same T and value_type
template <typename T, typename ValueType>
constexpr T operator-(quantity<T, ValueType> lhs, const quantity<T, ValueType> rhs) {
        return lhs -= rhs;
}

// Provides negation of the quantity
template <typename T, typename ValueType>
constexpr T operator-(const quantity<T, ValueType> val) {
        return T{-*val};
}

// Comparison of internal values of quantity
template <typename T, typename ValueType>
constexpr bool operator==(const quantity<T, ValueType> lhs, const quantity<T, ValueType> rhs) {
        return *lhs == *rhs;
}

// Comparasion of internal values
template <typename T, typename ValueType>
constexpr bool operator<(const quantity<T, ValueType> lhs, const quantity<T, ValueType> rhs) {
        return *lhs < *rhs;
}

// Multiplication of quantity by it's value_type
template <typename T, typename ValueType>
constexpr T operator*(quantity<T, ValueType> q, const ValueType val) {
        return q *= val;
}

// Division of quantity by it's value_type
template <typename T, typename ValueType>
constexpr T operator/(quantity<T, ValueType> q, const ValueType val) {
        return q /= val;
}

// Quantity with absolute value of internal value
template <typename T, typename ValueType>
constexpr T abs(const quantity<T, ValueType> q) {
        return T(std::abs(*q));
}

// Returns cosinus of the quantity as scalar
template <typename T, typename ValueType>
constexpr auto cos(const quantity<T, ValueType> u) {
        return std::cos(*u);
}

// Returns sinus of the quantity as scalar
template <typename T, typename ValueType>
constexpr auto sin(const quantity<T, ValueType> u) {
        return std::sin(*u);
}

// Quantity with maximum value of one of the quantities
template <typename T, typename ValueType>
constexpr T max(const quantity<T, ValueType> lh, const quantity<T, ValueType> rh) {
        return T(std::max(*lh, *rh));
}

// Quantity with minimum value of one of the quantities
template <typename T, typename ValueType>
constexpr T min(const quantity<T, ValueType> lh, const quantity<T, ValueType> rh) {
        return T(std::min(*lh, *rh));
}

//---------------------------------------------------------------------------

// Non-equality of quantites is negation of equality.
template <typename T, typename ValueType>
constexpr bool operator!=(const quantity<T, ValueType> lhs, const quantity<T, ValueType> rhs) {
        return !(lhs == rhs);
}

// Q1 > Q2 iff Q2 < Q1
template <typename T, typename ValueType>
constexpr bool operator>(const quantity<T, ValueType> lhs, const quantity<T, ValueType> rhs) {
        return rhs < lhs;
}
// Q1 <= Q2 iff !( Q2 > Q1 )
template <typename T, typename ValueType>
constexpr bool operator<=(const quantity<T, ValueType> lhs, const quantity<T, ValueType> rhs) {
        return !(lhs > rhs);
}
// Q1 >= Q2 iff !( Q2 < Q1 )
template <typename T, typename ValueType>
constexpr bool operator>=(const quantity<T, ValueType> lhs, const quantity<T, ValueType> rhs) {
        return !(lhs < rhs);
}
//---------------------------------------------------------------------------

// Multiplication of value_type by quantity returns quantity
template <typename T, typename ValueType>
constexpr T operator*(const ValueType val, const quantity<T, ValueType> q) {
        return q * val;
}
// Division of value_type by quantity returns quantity
template <typename T, typename ValueType>
constexpr ValueType operator/(const ValueType val, const quantity<T, ValueType> q) {
        return val / *q;
}

} // namespace emlabcpp

// The quantity has defined partital specialization of std::numeric_limits,
// works as is intuitive.
template <typename T, typename ValueType>
struct std::numeric_limits<emlabcpp::quantity<T, ValueType>> {
        constexpr static T lowest() { return T{std::numeric_limits<ValueType>::lowest()}; }
        constexpr static T min() { return T{std::numeric_limits<ValueType>::min()}; }
        constexpr static T max() { return T{std::numeric_limits<ValueType>::max()}; }
};

// Hash of quantity is hash of it's value and T::get_unit() xored.
template <typename T, typename ValueType>
struct std::hash<emlabcpp::quantity<T, ValueType>> {
        std::size_t operator()(const emlabcpp::quantity<T, ValueType> q) {
                // TODO: this should be rewritten
                // 'reverse' the prefix+unit info in bits and than xor it with number
                std::string unit = T::get_unit();
                return std::hash<ValueType>()(*q) ^ std::hash<std::string>()(unit);
        }
};
