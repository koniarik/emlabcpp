#include <cmath>
#include <limits>
#include <ratio>
#include <type_traits>

#pragma once

namespace emlabcpp {

/** Class representing generic quantity.
 *
 * Quantities are types which simply overlay basic numeric type (ValueType) and are tagged with some
 * unique type (Tag). The C++ type system prevents you from passing values of quantites of different
 * tags, unless explicitly stated!
 *
 * So if your function expects quantity with tag 'distance_tag', you can't pass it 'velocity_tag'.
 *
 * Only quantities of same Tag and ValueType are allowed following operations:
 * 	+=,-=
 * 	+,-
 * 	==, !=
 * 	<,>,>=,<=
 * 	abs, max, min
 * Quantity can be multiplied or divided by it's ValueType - /,*,/=,*=
 * Additionally, we support these operations over quantity:
 * 	cos, sin
 *
 * @param Tag template param that specifies the semantical meaning of the physical quantity.
 *
 * Credits should go to https://github.com/joboccara/NamedType as I inspired by project by this
 * blogger!
 */
template <typename Tag, typename ValueType = double>
class quantity final {
        ValueType value_;

      public:
        using tag        = Tag;
        using value_type = ValueType;

        constexpr quantity() noexcept : value_(0) {}

        // Default constructor used to create a physical quantity from value
        constexpr explicit quantity(ValueType val) noexcept : value_(val) {}

        // Const reference to the internal value
        constexpr ValueType operator*() const noexcept { return value_; }

        // Add other quantity of same tag and value_type
        constexpr quantity &operator+=(const quantity other) noexcept {
                value_ += *other;
                return *this;
        }

        // Subtract other quantity of same tag and value_type
        constexpr quantity &operator-=(const quantity other) noexcept {
                value_ -= *other;
                return *this;
        }

        // Divides quantity by it's value type
        constexpr quantity &operator/=(const ValueType val) noexcept {
                value_ /= val;
                return *this;
        }

        // Multiplies quantity by it's value type
        constexpr quantity &operator*=(const ValueType val) noexcept {
                value_ *= val;
                return *this;
        }

        // Provides explicit conversion of internal value to type T
        template <typename T>
        constexpr explicit operator T() const noexcept {
                return T(value_);
        }
};

// Sum of quantities with same tag and value_type
template <typename Tag, typename ValueType>
constexpr quantity<Tag, ValueType> operator+(quantity<Tag, ValueType>       lhs,
                                             const quantity<Tag, ValueType> rhs) {
        lhs += rhs;
        return lhs;
}

// Subtraction of quantities with same tag and value_type
template <typename Tag, typename ValueType>
constexpr quantity<Tag, ValueType> operator-(quantity<Tag, ValueType>       lhs,
                                             const quantity<Tag, ValueType> rhs) {
        lhs -= rhs;
        return lhs;
}

// Provides negation of the quantity
template <typename Tag, typename ValueType>
constexpr quantity<Tag, ValueType> operator-(const quantity<Tag, ValueType> val) {
        return quantity<Tag, ValueType>{-*val};
}

// Comparison of internal values of quantity
template <typename Tag, typename ValueType>
constexpr bool operator==(const quantity<Tag, ValueType> lhs, const quantity<Tag, ValueType> rhs) {
        return *lhs == *rhs;
}

// Comparasion of internal values
template <typename Tag, typename ValueType>
constexpr bool operator<(const quantity<Tag, ValueType> lhs, const quantity<Tag, ValueType> rhs) {
        return *lhs < *rhs;
}

// Multiplication of quantity by it's value_type
template <typename Tag, typename ValueType>
constexpr quantity<Tag, ValueType> operator*(quantity<Tag, ValueType> q, const ValueType val) {
        q *= val;
        return q;
}

// Division of quantity by it's value_type
template <typename Tag, typename ValueType>
constexpr quantity<Tag, ValueType> operator/(quantity<Tag, ValueType> q, const ValueType val) {
        q /= val;
        return q;
}

// Quantity with absolute value of internal value
template <typename Tag, typename ValueType>
constexpr quantity<Tag, ValueType> abs(const quantity<Tag, ValueType> q) {
        return quantity<Tag, ValueType>(std::abs(*q));
}

// Returns cosinus of the quantity - untagged
template <typename Tag, typename ValueType>
constexpr double cos(const quantity<Tag, ValueType> u) {
        return std::cos(*u);
}

// Returns sinus of the quantity - untagged
template <typename Tag, typename ValueType>
constexpr double sin(const quantity<Tag, ValueType> u) {
        return std::sin(*u);
}

// Quantity with maximum value of one of the quantities
template <typename Tag, typename ValueType>
constexpr quantity<Tag, ValueType> max(const quantity<Tag, ValueType> lh,
                                       const quantity<Tag, ValueType> rh) {
        return quantity<Tag, ValueType>(std::max(*lh, *rh));
}

// Quantity with minimum value of one of the quantities
template <typename Tag, typename ValueType>
constexpr quantity<Tag, ValueType> min(const quantity<Tag, ValueType> lh,
                                       const quantity<Tag, ValueType> rh) {
        return quantity<Tag, ValueType>(std::min(*lh, *rh));
}

//---------------------------------------------------------------------------

// Non-equality of quantites is negation of equality.
template <typename Tag, typename ValueType>
constexpr bool operator!=(const quantity<Tag, ValueType> lhs, const quantity<Tag, ValueType> rhs) {
        return !(lhs == rhs);
}

// Q1 > Q2 iff Q2 < Q1
template <typename Tag, typename ValueType>
constexpr bool operator>(const quantity<Tag, ValueType> lhs, const quantity<Tag, ValueType> rhs) {
        return rhs < lhs;
}
// Q1 <= Q2 iff !( Q2 > Q1 )
template <typename Tag, typename ValueType>
constexpr bool operator<=(const quantity<Tag, ValueType> lhs, const quantity<Tag, ValueType> rhs) {
        return !(lhs > rhs);
}
// Q1 >= Q2 iff !( Q2 < Q1 )
template <typename Tag, typename ValueType>
constexpr bool operator>=(const quantity<Tag, ValueType> lhs, const quantity<Tag, ValueType> rhs) {
        return !(lhs < rhs);
}
//---------------------------------------------------------------------------

// Multiplication of value_type by quantity returns quantity
template <typename Tag, typename ValueType>
constexpr quantity<Tag, ValueType> operator*(const ValueType                val,
                                             const quantity<Tag, ValueType> q) {
        return q * val;
}
// Division of value_type by quantity returns quantity
template <typename Tag, typename ValueType>
constexpr ValueType operator/(const ValueType val, const quantity<Tag, ValueType> q) {
        return val / *q;
}

} // namespace emlabcpp

// The quantity has defined partital specialization of std::numeric_limits,
// works as is intuitive.
template <typename Tag, typename ValueType>
class std::numeric_limits<emlabcpp::quantity<Tag, ValueType>> {
      public:
        constexpr static emlabcpp::quantity<Tag, ValueType> lowest() {
                return emlabcpp::quantity<Tag, ValueType>{std::numeric_limits<ValueType>::lowest()};
        }
        constexpr static emlabcpp::quantity<Tag, ValueType> min() {
                return emlabcpp::quantity<Tag, ValueType>{std::numeric_limits<ValueType>::min()};
        }
        constexpr static emlabcpp::quantity<Tag, ValueType> max() {
                return emlabcpp::quantity<Tag, ValueType>{std::numeric_limits<ValueType>::max()};
        }
};

// Hash of quantity is hash of it's value and Tag::get_unit() xored.
template <typename Tag, typename ValueType>
struct std::hash<emlabcpp::quantity<Tag, ValueType>> {
        std::size_t operator()(const emlabcpp::quantity<Tag, ValueType> q) {
                // TODO: this should be rewritten
                // 'reverse' the prefix+unit info in bits and than xor it with number
                std::string unit = Tag::get_unit();
                return std::hash<ValueType>()(*q) ^ std::hash<std::string>()(unit);
        }
};
