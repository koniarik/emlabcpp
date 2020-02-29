
#include "emlabcpp/quantity.h"

#pragma once

namespace emlabcpp {

// Physical quantity represents all physical units defined using the International System of
// Units and more. The idea is that each used unit, is either one of the seven basic units, or
// defined as some combination of them.  The combination is multiplication of exponents of basic
// units.
//
// So, velocity is distance per time - m*s^-1.
//
// We defined each physical unit by using a template, that has exponent of each basic unit as
// exponent. This makes it possible to specify any unit using this class.
//
// We expand this by providing two additional 'basic' units - angle (which is handy for us) and
// byte.
//
// Given that we have the exponents of basic units as integers in the type, we can write generic
// multiplication and division between all possible templates.
//
// This trick is inspired by the haskell's dimensional library which does the same.
//
template <int l, int mass, int t, int current, int temp, int mol, int li, int angle, int byte>
struct physical_quantity
    : quantity<physical_quantity<l, mass, t, current, temp, mol, li, angle, byte>, float> {

        using quantity<physical_quantity<l, mass, t, current, temp, mol, li, angle, byte>,
                       float>::quantity;

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
using unitless            = physical_quantity<0, 0, 0, 0, 0, 0, 0, 0, 0>;
using length              = physical_quantity<1, 0, 0, 0, 0, 0, 0, 0, 0>;
using mass                = physical_quantity<0, 1, 0, 0, 0, 0, 0, 0, 0>;
using timeq               = physical_quantity<0, 0, 1, 0, 0, 0, 0, 0, 0>;
using current             = physical_quantity<0, 0, 0, 1, 0, 0, 0, 0, 0>;
using temp                = physical_quantity<0, 0, 0, 0, 1, 0, 0, 0, 0>;
using amount_of_substance = physical_quantity<0, 0, 0, 0, 0, 1, 0, 0, 0>;
using luminous_intensity  = physical_quantity<0, 0, 0, 0, 0, 0, 1, 0, 0>;
using angle               = physical_quantity<0, 0, 0, 0, 0, 0, 0, 1, 0>;
using byte                = physical_quantity<0, 0, 0, 0, 0, 0, 0, 0, 1>;
using angular_velocity    = physical_quantity<0, 0, -1, 0, 0, 0, 0, 1, 0>;
using area                = physical_quantity<2, 0, 0, 0, 0, 0, 0, 0, 0>;
using volume              = physical_quantity<3, 0, 0, 0, 0, 0, 0, 0, 0>;
using velocity            = physical_quantity<1, 0, -1, 0, 0, 0, 0, 0, 0>;
using frequency           = physical_quantity<0, 0, -1, 0, 0, 0, 0, 0, 0>;
using force               = physical_quantity<1, 1, -2, 0, 0, 0, 0, 0, 0>;
using power               = physical_quantity<2, 1, -3, 0, 0, 0, 0, 0, 0>;
using voltage             = physical_quantity<2, 1, -3, -1, 0, 0, 0, 0, 0>;
using resistance          = physical_quantity<2, 1, -3, -2, 0, 0, 0, 0, 0>;
using distance            = length;
using radius              = length;

// Constants of units that are relevant for us
constexpr angle PI = angle{3.14159265358979323846f};

// Multiplication of quantities of physical_quantiy multiplies the internal
// values and the result is a type, where the exponents of each side of the
// multiplication are summed.
template <int l0, int mass0, int t0, int curr0, int temp0, int mol0, int li0, int angle0, int byte0,
          int l1, int mass1, int t1, int curr1, int temp1, int mol1, int li1, int angle1, int byte1>
constexpr auto
operator*(physical_quantity<l0, mass0, t0, curr0, temp0, mol0, li0, angle0, byte0> lh,
          physical_quantity<l1, mass1, t1, curr1, temp1, mol1, li1, angle1, byte1> rh) {
        return physical_quantity<l0 + l1, mass0 + mass1, t0 + t1, curr0 + curr1, temp0 + temp1,
                                 mol0 + mol1, li0 + li1, angle0 + angle1, byte0 + byte1>{(*lh) *
                                                                                         (*rh)};
}

// Divison of quantities of physical_quantiy divides the internal values and
// the result is a type, where the exponents of each side of the multiplication
// are subtracted.
template <int l0, int mass0, int t0, int curr0, int temp0, int mol0, int li0, int angle0, int byte0,
          int l1, int mass1, int t1, int curr1, int temp1, int mol1, int li1, int angle1, int byte1>
constexpr auto
operator/(physical_quantity<l0, mass0, t0, curr0, temp0, mol0, li0, angle0, byte0> lh,
          physical_quantity<l1, mass1, t1, curr1, temp1, mol1, li1, angle1, byte1> rh) {
        return physical_quantity<l0 - l1, mass0 - mass1, t0 - t1, curr0 - curr1, temp0 - temp1,
                                 mol0 - mol1, li0 - li1, angle0 - angle1, byte0 - byte1>{(*lh) /
                                                                                         (*rh)};
}

// Square root of physical quantity is square root of it's value and the
// exponents are divided in half.
template <int l, int mass, int t, int curr, int temp, int mol, int li, int angle, int byte>
constexpr auto sqrt(physical_quantity<l, mass, t, curr, temp, mol, li, angle, byte> val) {
        return physical_quantity<l / 2, mass / 2, t / 2, curr / 2, temp / 2, mol / 2, li / 2,
                                 angle / 2, byte / 2>{float{std::sqrt(*val)}};
}

} // namespace emlabcpp

template <int l, int mass, int t, int curr, int temp, int mol, int li, int angle, int byte>
struct std::numeric_limits<
    emlabcpp::physical_quantity<l, mass, t, curr, temp, mol, li, angle, byte>>
    : std::numeric_limits<emlabcpp::quantity<
          emlabcpp::physical_quantity<l, mass, t, curr, temp, mol, li, angle, byte>, float>> {};

template <int l, int mass, int t, int curr, int temp, int mol, int li, int angle, int byte>
struct std::hash<emlabcpp::physical_quantity<l, mass, t, curr, temp, mol, li, angle, byte>>
    : std::hash<emlabcpp::quantity<
          emlabcpp::physical_quantity<l, mass, t, curr, temp, mol, li, angle, byte>, float>> {};
