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
#include "emlabcpp/quantity.h"

#include <string>

#pragma once

namespace emlabcpp
{

/// physical_quantity represents all physical units defined using the International System of
/// Units and more. The idea is that each used unit, is either one of the seven basic units, or
/// defined as some combination of them.  The combination is multiplication of exponents of basic
/// units.
///
/// So, velocity is distance per time - m*s^-1.
///
/// We defined each physical unit by using a template, that has exponent of each basic unit as
/// exponent. This makes it possible to specify any unit using this class - as combination of basic
/// units.
///
/// We expand this by providing two additional 'basic' units - angle (which is handy for us) and
/// byte.
///
/// Given that we have the exponents of basic units as integers in the type, we can write generic
/// multiplication and division between all possible templates.
///
/// This trick is inspired by the haskell's dimensional library which does the same.
///
template < int Len, int Mass, int Time, int Current, int Temp, int Mol, int Li, int Angle, int Byte >
struct physical_quantity
  : quantity< physical_quantity< Len, Mass, Time, Current, Temp, Mol, Li, Angle, Byte >, float >
{

        using quantity<
            physical_quantity< Len, Mass, Time, Current, Temp, Mol, Li, Angle, Byte >,
            float >::quantity;

        static std::string get_unit()
        {
                auto seg = []( std::string unit, int i ) -> std::string {
                        if ( i == 0 ) {
                                return "";
                        }
                        if ( i == 1 ) {
                                return unit;
                        }
                        return unit + "^" + std::to_string( i );
                };
                return seg( "m", Len ) + seg( "g", Mass ) + seg( "s", Time ) + seg( "A", Current ) +
                       seg( "K", Temp ) + seg( "mol", Mol ) + seg( "cd", Li ) +
                       seg( "rad", Angle ) + seg( "B", Byte );
        }
};

///@{
/// Type alieases of physical quantity for used quantities.
using unitless            = physical_quantity< 0, 0, 0, 0, 0, 0, 0, 0, 0 >;
using length              = physical_quantity< 1, 0, 0, 0, 0, 0, 0, 0, 0 >;
using mass                = physical_quantity< 0, 1, 0, 0, 0, 0, 0, 0, 0 >;
using timeq               = physical_quantity< 0, 0, 1, 0, 0, 0, 0, 0, 0 >;
using current             = physical_quantity< 0, 0, 0, 1, 0, 0, 0, 0, 0 >;
using temp                = physical_quantity< 0, 0, 0, 0, 1, 0, 0, 0, 0 >;
using amount_of_substance = physical_quantity< 0, 0, 0, 0, 0, 1, 0, 0, 0 >;
using luminous_intensity  = physical_quantity< 0, 0, 0, 0, 0, 0, 1, 0, 0 >;
using angle               = physical_quantity< 0, 0, 0, 0, 0, 0, 0, 1, 0 >;
using byte                = physical_quantity< 0, 0, 0, 0, 0, 0, 0, 0, 1 >;
using acceleration        = physical_quantity< 1, 0, -2, 0, 0, 0, 0, 0, 0 >;
using angular_velocity    = physical_quantity< 0, 0, -1, 0, 0, 0, 0, 1, 0 >;
using area                = physical_quantity< 2, 0, 0, 0, 0, 0, 0, 0, 0 >;
using volume              = physical_quantity< 3, 0, 0, 0, 0, 0, 0, 0, 0 >;
using velocity            = physical_quantity< 1, 0, -1, 0, 0, 0, 0, 0, 0 >;
using frequency           = physical_quantity< 0, 0, -1, 0, 0, 0, 0, 0, 0 >;
using force               = physical_quantity< 1, 1, -2, 0, 0, 0, 0, 0, 0 >;
using power               = physical_quantity< 2, 1, -3, 0, 0, 0, 0, 0, 0 >;
using voltage             = physical_quantity< 2, 1, -3, -1, 0, 0, 0, 0, 0 >;
using resistance          = physical_quantity< 2, 1, -3, -2, 0, 0, 0, 0, 0 >;
using distance            = length;
using radius              = length;
///@}

/// Constants of units that are relevant for us
constexpr angle pi = angle{ 3.14159265358979323846f };

/// Multiplication of physical_quantity multiplies the internal
/// values and the result is a type, where the exponents of each side of the
/// multiplication are added together.
template <
    int Len0,
    int Mass0,
    int Time0,
    int Current0,
    int Temp0,
    int Mol0,
    int Li0,
    int Angle0,
    int Byte0,
    int Len1,
    int Mass1,
    int Time1,
    int Current1,
    int Temp1,
    int Mol1,
    int Li1,
    int Angle1,
    int Byte1 >
constexpr auto operator*(
    physical_quantity< Len0, Mass0, Time0, Current0, Temp0, Mol0, Li0, Angle0, Byte0 > lh,
    physical_quantity< Len1, Mass1, Time1, Current1, Temp1, Mol1, Li1, Angle1, Byte1 > rh )
{
        return physical_quantity<
            Len0 + Len1,
            Mass0 + Mass1,
            Time0 + Time1,
            Current0 + Current1,
            Temp0 + Temp1,
            Mol0 + Mol1,
            Li0 + Li1,
            Angle0 + Angle1,
            Byte0 + Byte1 >{ ( *lh ) * ( *rh ) };
}

/// Divison of physical_quantiy divides the internal values and
/// the result is a type, where the exponents of each side of the multiplication
/// are subtracted.
template <
    int Len0,
    int Mass0,
    int Time0,
    int Current0,
    int Temp0,
    int Mol0,
    int Li0,
    int Angle0,
    int Byte0,
    int Len1,
    int Mass1,
    int Time1,
    int Current1,
    int Temp1,
    int Mol1,
    int Li1,
    int Angle1,
    int Byte1 >
constexpr auto operator/(
    physical_quantity< Len0, Mass0, Time0, Current0, Temp0, Mol0, Li0, Angle0, Byte0 > lh,
    physical_quantity< Len1, Mass1, Time1, Current1, Temp1, Mol1, Li1, Angle1, Byte1 > rh )
{
        return physical_quantity<
            Len0 - Len1,
            Mass0 - Mass1,
            Time0 - Time1,
            Current0 - Current1,
            Temp0 - Temp1,
            Mol0 - Mol1,
            Li0 - Li1,
            Angle0 - Angle1,
            Byte0 - Byte1 >{ ( *lh ) / ( *rh ) };
}

/// Square root of physical quantity is square root of it's value and the
/// exponents are divided in half.
template < int Len, int Mass, int Time, int Current, int Temp, int Mol, int Li, int Angle, int Byte >
constexpr auto sqrt( physical_quantity< Len, Mass, Time, Current, Temp, Mol, Li, Angle, Byte > val )
{
        static_assert(
            Len % 2 == 0 && Mass % 2 == 0 && Time % 2 == 0 && Current % 2 == 0 && Temp % 2 == 0 &&
                Mol % 2 == 0 && Li % 2 == 0 && Angle % 2 == 0 && Byte % 2 == 0,
            "sqrt() over physical_quantity can be used only if all of the units are power of 2" );
        return physical_quantity<
            Len / 2,
            Mass / 2,
            Time / 2,
            Current / 2,
            Temp / 2,
            Mol / 2,
            Li / 2,
            Angle / 2,
            Byte / 2 >{ float{ std::sqrt( *val ) } };
}

/// Power of physical quantity is power of root of it's value and the exponents are multiplied by
/// the value.

template <
    int Power,
    int Len,
    int Mass,
    int Time,
    int Current,
    int Temp,
    int Mol,
    int Li,
    int Angle,
    int Byte >
constexpr auto pow( physical_quantity< Len, Mass, Time, Current, Temp, Mol, Li, Angle, Byte > val )
{
        return physical_quantity<
            Len * Power,
            Mass * Power,
            Time * Power,
            Current * Power,
            Temp * Power,
            Mol * Power,
            Li * Power,
            Angle * Power,
            Byte * Power >{ static_cast< float >( std::pow( *val, Power ) ) };
}

template <
    ostreamlike Stream,
    int         Len,
    int         Mass,
    int         Time,
    int         Current,
    int         Temp,
    int         Mol,
    int         Li,
    int         Angle,
    int         Byte >
auto& operator<<(
    Stream&                                                                          os,
    const physical_quantity< Len, Mass, Time, Current, Temp, Mol, Li, Angle, Byte >& q )
{
        return os << *q
                  << physical_quantity< Len, Mass, Time, Current, Temp, Mol, Li, Angle, Byte >::
                         get_unit();
}

}  // namespace emlabcpp
