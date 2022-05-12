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
#include "emlabcpp/types/base.h"

#include <concepts>

#pragma once

namespace emlabcpp
{

template < typename T >
concept arithmetic_operators = requires( T a, T b )
{
        {
                a + b
                } -> std::convertible_to< T >;
        {
                a - b
                } -> std::convertible_to< T >;
        {
                a / b
                } -> std::convertible_to< T >;
        {
                a* b
                } -> std::convertible_to< T >;
};

template < typename T >
concept arithmetic_assignment = requires( T a, T b )
{
        { a += b };
        { a -= b };
        { a /= b };
        { a *= b };
};

template < typename T >
concept arithmetic_like = arithmetic_operators< T > && arithmetic_assignment< T >;

template < typename T >
concept arithmetic = std::integral< T > || std::floating_point< T >;

template < typename T >
concept gettable_container = requires( T a )
{
        {
                std::tuple_size< std::decay_t< T > >::value
                } -> std::convertible_to< std::size_t >;
};

/// so, std::ranges::range is meh because it expects return of begin() being input_output_iterator,
/// which has to be def.constructible
template < typename T >
concept range_container = (
                              requires( T a ) { begin( a ); } ||
                              requires( T a ) { std::begin( a ); } ) &&
                          (
                              requires( T a ) { end( a ); } || requires( T a ) { std::end( a ); } );

template < typename T >
concept container = range_container< T > || gettable_container< T >;

template < typename T >
concept referenceable_container = is_view< T >::value ||
    ( range_container< T > && !std::is_rvalue_reference_v< T > );

template < typename T >
concept static_sized = requires( T a )
{
        {
                std::tuple_size< std::decay_t< T > >::value
                } -> std::convertible_to< std::size_t >;
};

template < typename UnaryFunction, typename Container >
concept container_invocable = requires( Container cont, UnaryFunction f )
{
        f( *cont.begin() );
}
|| requires( Container cont )
{
        std::tuple_size< std::decay_t< Container > >::value == 0;
}
|| requires( Container cont, UnaryFunction f )
{
        /// this has to come after the size check, as gcc 10.2 will faill to compile the code using
        /// this concept otherwise. If container is std::tuple<> and this check comes before the
        /// size one, it fails on std::get<0> being not compailable.
        f( std::get< 0 >( cont ) );
};

template < typename UnaryFunction, typename ReturnValue, typename... Args >
concept invocable_returning = requires( UnaryFunction f, Args... args )
{
        {
                f( args... )
                } -> std::same_as< ReturnValue >;
};

namespace detail
{
        template < typename Stream, typename T >
        concept directly_streamable_for = requires( Stream os, T val )
        {
                {
                        os.operator<<( val )
                        } -> std::same_as< Stream& >;
        };
}  // namespace detail

template < typename T >
concept ostreamlike = requires( T val )
{
        {
                val.good()
                } -> std::same_as< bool >;
        {
                val.bad()
                } -> std::same_as< bool >;
        bool( val );
        typename T::char_type;
        detail::directly_streamable_for< T, uint8_t >;
        detail::directly_streamable_for< T, uint16_t >;
        detail::directly_streamable_for< T, uint32_t >;
        detail::directly_streamable_for< T, int8_t >;
        detail::directly_streamable_for< T, int16_t >;
        detail::directly_streamable_for< T, int32_t >;
        detail::directly_streamable_for< T, float >;
        detail::directly_streamable_for< T, double >;
        detail::directly_streamable_for< T, bool >;
        detail::directly_streamable_for< T, const void* >;
        detail::directly_streamable_for< T, std::nullptr_t >;
};

}  // namespace emlabcpp
