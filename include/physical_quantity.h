
#include "quantity.h"

#pragma once

namespace emlabcpp {

// Physical quantity tag represents all physical units defined using the International System of
// Units and more. The idea is that each used unit, is either one of the seven basic units, or
// defined as some combination of them.  The combination is multiplication of exponents of basic
// units.
//
// So, velocity is distance per time - m*s^-1.
//
// We defined each physical unit by defining a tag, that has exponent of each basic unit as
// exponent. This makes it possible to specify any unit using this tag.
//
// We expand this by providing two additional 'basic' units - angle (which is handy for us) and
// byte.
//
// Given that we have the exponents of basic units as integers in the type, we can write generic
// multiplication and division between all possible tags.
//
// This trick is inspired by the haskell's dimensional library which does the same.
//
template <int l, int mass, int t, int current, int temp, int mol, int li, int angle, int byte>
struct physical_quantity_tag {
        static std::string get_unit() {
                auto seg = [](std::string unit, int i) -> std::string {
                        if (i == 0) {
                                return std::string();
                        }
                        if (i == 1) {
                                return unit;
                        }
                        return unit + "^" + std::to_string(i);
                };
                return seg("m", l) + seg("g", mass) + seg("s", t) + seg("A", current) +
                       seg("K", temp) + seg("mol", mol) + seg("cd", li) + seg("rad", angle) +
                       seg("B", byte);
        }
};

// Table of alieses for most used physical units
using unitless            = quantity<physical_quantity_tag<0, 0, 0, 0, 0, 0, 0, 0, 0>, float>;
using length              = quantity<physical_quantity_tag<1, 0, 0, 0, 0, 0, 0, 0, 0>, float>;
using mass                = quantity<physical_quantity_tag<0, 1, 0, 0, 0, 0, 0, 0, 0>, float>;
using timeq               = quantity<physical_quantity_tag<0, 0, 1, 0, 0, 0, 0, 0, 0>, float>;
using current             = quantity<physical_quantity_tag<0, 0, 0, 1, 0, 0, 0, 0, 0>, float>;
using temp                = quantity<physical_quantity_tag<0, 0, 0, 0, 1, 0, 0, 0, 0>, float>;
using amount_of_substance = quantity<physical_quantity_tag<0, 0, 0, 0, 0, 1, 0, 0, 0>, float>;
using luminous_intensity  = quantity<physical_quantity_tag<0, 0, 0, 0, 0, 0, 1, 0, 0>, float>;
using angle               = quantity<physical_quantity_tag<0, 0, 0, 0, 0, 0, 0, 1, 0>, float>;
using byte                = quantity<physical_quantity_tag<0, 0, 0, 0, 0, 0, 0, 0, 1>, float>;
using angular_velocity    = quantity<physical_quantity_tag<0, 0, -1, 0, 0, 0, 0, 1, 0>, float>;
using area                = quantity<physical_quantity_tag<2, 0, 0, 0, 0, 0, 0, 0, 0>, float>;
using volume              = quantity<physical_quantity_tag<3, 0, 0, 0, 0, 0, 0, 0, 0>, float>;
using velocity            = quantity<physical_quantity_tag<1, 0, -1, 0, 0, 0, 0, 0, 0>, float>;
using frequency           = quantity<physical_quantity_tag<0, 0, -1, 0, 0, 0, 0, 0, 0>, float>;
using force               = quantity<physical_quantity_tag<1, 1, -2, 0, 0, 0, 0, 0, 0>, float>;
using power               = quantity<physical_quantity_tag<2, 1, -3, 0, 0, 0, 0, 0, 0>, float>;
using voltage             = quantity<physical_quantity_tag<2, 1, -3, -1, 0, 0, 0, 0, 0>, float>;
using resistance          = quantity<physical_quantity_tag<2, 1, -3, -2, 0, 0, 0, 0, 0>, float>;
using distance            = length;
using radius              = length;

// Constants of units that are relevant for us
constexpr angle PI = angle{float(std::acos(-1))};

// Multiplication of quantities of physical_quantiy_tag multiplies the internal
// values and the result is a type, where the exponents of each side of the
// multiplication are summed.
template <int l0, int mass0, int t0, int curr0, int temp0, int mol0, int li0, int angle0, int byte0,
          int l1, int mass1, int t1, int curr1, int temp1, int mol1, int li1, int angle1, int byte1,
          typename ValueType>
constexpr auto
operator*(quantity<physical_quantity_tag<l0, mass0, t0, curr0, temp0, mol0, li0, angle0, byte0>,
                   ValueType>
              lh,
          quantity<physical_quantity_tag<l1, mass1, t1, curr1, temp1, mol1, li1, angle1, byte1>,
                   ValueType>
              rh) {
        return quantity<
            physical_quantity_tag<l0 + l1, mass0 + mass1, t0 + t1, curr0 + curr1, temp0 + temp1,
                                  mol0 + mol1, li0 + li1, angle0 + angle1, byte0 + byte1>,
            ValueType>{(*lh) * (*rh)};
}

// Divison of quantities of physical_quantiy_tag divides the internal values and
// the result is a type, where the exponents of each side of the multiplication
// are subtracted.
template <int l0, int mass0, int t0, int curr0, int temp0, int mol0, int li0, int angle0, int byte0,
          int l1, int mass1, int t1, int curr1, int temp1, int mol1, int li1, int angle1, int byte1,
          typename ValueType>
constexpr auto
operator/(quantity<physical_quantity_tag<l0, mass0, t0, curr0, temp0, mol0, li0, angle0, byte0>,
                   ValueType>
              lh,
          quantity<physical_quantity_tag<l1, mass1, t1, curr1, temp1, mol1, li1, angle1, byte1>,
                   ValueType>
              rh) {
        return quantity<
            physical_quantity_tag<l0 - l1, mass0 - mass1, t0 - t1, curr0 - curr1, temp0 - temp1,
                                  mol0 - mol1, li0 - li1, angle0 - angle1, byte0 - byte1>,
            ValueType>{(*lh) / (*rh)};
}

// Square root of physical quantity is square root of it's value and the
// exponents are divided in half.
template <int l, int mass, int t, int curr, int temp, int mol, int li, int angle, int byte,
          typename ValueType>
constexpr auto
sqrt(quantity<physical_quantity_tag<l, mass, t, curr, temp, mol, li, angle, byte>, ValueType> val) {
        return quantity<physical_quantity_tag<l / 2, mass / 2, t / 2, curr / 2, temp / 2, mol / 2,
                                              li / 2, angle / 2, byte / 2>,
                        ValueType>{ValueType{std::sqrt(*val)}};
}

} // namespace emlabcpp
