#include <cmath>
#include <functional>
#include <limits>
#include <ratio>
#include <type_traits>

#pragma once

namespace emlabcpp {

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
template <typename Derived, typename ValueType = float>
class quantity {
        ValueType value_;

        [[nodiscard]] Derived &      impl() { return static_cast<Derived &>(*this); }
        [[nodiscard]] Derived const &impl() const { return static_cast<Derived const &>(*this); }

      public:
        using value_type = ValueType;

        constexpr quantity() noexcept : value_(0) {}

        /// Default constructor used to create a physical quantity from value
        constexpr explicit quantity(ValueType val) noexcept : value_(val) {}

        /// Const reference to the internal value
        constexpr ValueType operator*() const noexcept { return value_; }

        /// Add other quantity of same Derived and value_type
        constexpr Derived &operator+=(const quantity other) noexcept {
                value_ += *other;
                return impl();
        }

        /// Subtract other quantity of same Derived and value_type
        constexpr Derived &operator-=(const quantity other) noexcept {
                value_ -= *other;
                return impl();
        }

        /// Divides quantity by it's value type
        constexpr Derived &operator/=(const ValueType val) noexcept {
                value_ /= val;
                return impl();
        }

        /// Multiplies quantity by it's value type
        constexpr Derived &operator*=(const ValueType val) noexcept {
                value_ *= val;
                return impl();
        }

        /// Provides explicit conversion of internal value to type U
        template <typename U>
        constexpr explicit operator U() const noexcept {
                return U(value_);
        }

        static std::string get_unit() { return ""; }
};

/// Sum of quantities with same Derived and value_type
template <typename Derived, typename ValueType>
constexpr Derived operator+(quantity<Derived, ValueType>       lhs,
                            const quantity<Derived, ValueType> rhs) {
        return lhs += rhs;
}

/// Subtraction of quantities with same Derived and value_type
template <typename Derived, typename ValueType>
constexpr Derived operator-(quantity<Derived, ValueType>       lhs,
                            const quantity<Derived, ValueType> rhs) {
        return lhs -= rhs;
}

/// Provides negation of the quantity
template <typename Derived, typename ValueType>
constexpr Derived operator-(const quantity<Derived, ValueType> val) {
        return Derived{-*val};
}

/// Comparison of internal values of quantity
template <typename Derived, typename ValueType>
constexpr bool operator==(const quantity<Derived, ValueType> lhs,
                          const quantity<Derived, ValueType> rhs) {
        return *lhs == *rhs;
}

/// Comparasion of internal values
template <typename Derived, typename ValueType>
constexpr bool operator<(const quantity<Derived, ValueType> lhs,
                         const quantity<Derived, ValueType> rhs) {
        return *lhs < *rhs;
}

/// Multiplication of quantity by it's value_type
template <typename Derived, typename ValueType>
constexpr Derived operator*(quantity<Derived, ValueType> q, const ValueType val) {
        return q *= val;
}

/// Division of quantity by it's value_type
template <typename Derived, typename ValueType>
constexpr Derived operator/(quantity<Derived, ValueType> q, const ValueType val) {
        return q /= val;
}

/// Quantity with absolute value of internal value
template <typename Derived, typename ValueType>
constexpr Derived abs(const quantity<Derived, ValueType> q) {
        return Derived(std::abs(*q));
}

/// Returns cosinus of the quantity as scalar
template <typename Derived, typename ValueType>
constexpr auto cos(const quantity<Derived, ValueType> u) {
        return std::cos(*u);
}

/// Returns sinus of the quantity as scalar
template <typename Derived, typename ValueType>
constexpr auto sin(const quantity<Derived, ValueType> u) {
        return std::sin(*u);
}

/// Quantity with maximum value of one of the quantities
template <typename Derived, typename ValueType>
constexpr Derived max(const quantity<Derived, ValueType> lh,
                      const quantity<Derived, ValueType> rh) {
        return Derived(std::max(*lh, *rh));
}

/// Quantity with minimum value of one of the quantities
template <typename Derived, typename ValueType>
constexpr Derived min(const quantity<Derived, ValueType> lh,
                      const quantity<Derived, ValueType> rh) {
        return Derived(std::min(*lh, *rh));
}

//---------------------------------------------------------------------------

/// Non-equality of quantites is negation of equality.
template <typename Derived, typename ValueType>
constexpr bool operator!=(const quantity<Derived, ValueType> lhs,
                          const quantity<Derived, ValueType> rhs) {
        return !(lhs == rhs);
}

/// Q1 > Q2 iff Q2 < Q1
template <typename Derived, typename ValueType>
constexpr bool operator>(const quantity<Derived, ValueType> lhs,
                         const quantity<Derived, ValueType> rhs) {
        return rhs < lhs;
}
/// Q1 <= Q2 iff !( Q2 > Q1 )
template <typename Derived, typename ValueType>
constexpr bool operator<=(const quantity<Derived, ValueType> lhs,
                          const quantity<Derived, ValueType> rhs) {
        return !(lhs > rhs);
}
/// Q1 >= Q2 iff !( Q2 < Q1 )
template <typename Derived, typename ValueType>
constexpr bool operator>=(const quantity<Derived, ValueType> lhs,
                          const quantity<Derived, ValueType> rhs) {
        return !(lhs < rhs);
}
//---------------------------------------------------------------------------

/// Multiplication of value_type by quantity returns quantity
template <typename Derived, typename ValueType>
constexpr Derived operator*(const ValueType val, const quantity<Derived, ValueType> q) {
        return q * val;
}
/// Division of value_type by quantity returns quantity
template <typename Derived, typename ValueType>
constexpr ValueType operator/(const ValueType val, const quantity<Derived, ValueType> q) {
        return val / *q;
}

} // namespace emlabcpp

/// The quantity has defined partital specialization of std::numeric_limits,
/// works as std::numeric_limits<ValueType>;
template <typename Derived, typename ValueType>
struct std::numeric_limits<emlabcpp::quantity<Derived, ValueType>> {
        constexpr static Derived lowest() {
                return Derived{std::numeric_limits<ValueType>::lowest()};
        }
        constexpr static Derived min() { return Derived{std::numeric_limits<ValueType>::min()}; }
        constexpr static Derived max() { return Derived{std::numeric_limits<ValueType>::max()}; }
};

/// Hash of quantity is hash of it's value and Derived::get_unit() xored.
template <typename Derived, typename ValueType>
struct std::hash<emlabcpp::quantity<Derived, ValueType>> {
        std::size_t operator()(const emlabcpp::quantity<Derived, ValueType> q) {
                // TODO: this should be rewritten
                // 'reverse' the prefix+unit info in bits and than xor it with number
                std::string unit = Derived::get_unit();
                return std::hash<ValueType>()(*q) ^ std::hash<std::string>()(unit);
        }
};
