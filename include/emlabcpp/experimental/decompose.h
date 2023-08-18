///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
/// and associated documentation files (the "Software"), to deal in the Software without
/// restriction, including without limitation the rights to use, copy, modify, merge, publish,
/// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or
/// substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
/// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
/// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#pragma once

#include "emlabcpp/concepts.h"

#include <tuple>

namespace emlabcpp
{

// thanks to the great presentation: https://www.youtube.com/watch?v=abdeAew3gmQ (Antony Polukhin)
namespace detail
{

        static constexpr std::size_t max_decompose_count = 16;

        template < std::size_t >
        struct decompose_anything
        {
                template < typename T >
                operator T() const;
        };

        template < class T, std::size_t I0, std::size_t... I >
        constexpr auto decompose_count_impl( int& out, std::index_sequence< I0, I... > )
            -> std::add_pointer_t<
                decltype( T{ decompose_anything< I0 >{}, decompose_anything< I >{}... } ) >
        {
                out = sizeof...( I ) + 1;
                return nullptr;
        }

        template < class T, std::size_t... I >
        constexpr void* decompose_count_impl( int& out, std::index_sequence< I... > )
        {
                if constexpr ( sizeof...( I ) > 0 ) {
                        decompose_count_impl< T >(
                            out, std::make_index_sequence< sizeof...( I ) - 1 >{} );
                } else if constexpr ( std::is_default_constructible_v< T > ) {
                        out = 0;
                } else {
                        out = -1;
                }
                return nullptr;
        }

        template < typename T >
        constexpr int decompose_count()
        {
                int c = -2;
                decompose_count_impl< std::decay_t< T > >(
                    c, std::make_index_sequence< max_decompose_count >{} );
                return c;
        }

}  // namespace detail

template < typename T >
concept decomposable = std::is_class_v< std::decay_t< T > > && !
gettable_container< T > && ( detail::decompose_count< T >() >= 0 );

#define EMLABCPP_GENERATE_DECOMPOSE( ID, ... )                                                    \
        template < typename T >                                                                   \
        concept decomposable_##ID = decomposable< T >&& detail::decompose_count< T >() == ( ID ); \
                                                                                                  \
        template < decomposable_##ID T >                                                          \
        constexpr auto decompose( T&& item )                                                      \
        {                                                                                         \
                if constexpr ( !std::is_lvalue_reference_v< T > ) {                               \
                        auto&& [__VA_ARGS__] = std::forward< T >( item );                         \
                        return std::make_tuple( __VA_ARGS__ );                                    \
                } else {                                                                          \
                        auto& [__VA_ARGS__] = item;                                               \
                        return std::tie( __VA_ARGS__ );                                           \
                }                                                                                 \
        }

template < typename T >
concept decomposable_0 = decomposable< T > && ( detail::decompose_count< T >() == 0 );

template < decomposable_0 T >
constexpr std::tuple<> decompose( T&& )
{
        return {};
}

template < typename T, typename Tuple >
constexpr T compose( Tuple tpl )
{
        return std::apply(
            []< typename... Args >( Args&&... args ) {
                    return T{ std::forward< Args >( args )... };
            },
            std::move( tpl ) );
}

EMLABCPP_GENERATE_DECOMPOSE( 1, a0 )
EMLABCPP_GENERATE_DECOMPOSE( 2, a0, a1 )
EMLABCPP_GENERATE_DECOMPOSE( 3, a0, a1, a2 )
EMLABCPP_GENERATE_DECOMPOSE( 4, a0, a1, a2, a3 )
EMLABCPP_GENERATE_DECOMPOSE( 5, a0, a1, a2, a3, a4 )
EMLABCPP_GENERATE_DECOMPOSE( 6, a0, a1, a2, a3, a4, a5 )
EMLABCPP_GENERATE_DECOMPOSE( 7, a0, a1, a2, a3, a4, a5, a6 )
EMLABCPP_GENERATE_DECOMPOSE( 8, a0, a1, a2, a3, a4, a5, a6, a7 )
EMLABCPP_GENERATE_DECOMPOSE( 9, a0, a1, a2, a3, a4, a5, a6, a7, a8 )
EMLABCPP_GENERATE_DECOMPOSE( 10, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9 )
EMLABCPP_GENERATE_DECOMPOSE( 11, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10 )
EMLABCPP_GENERATE_DECOMPOSE( 12, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11 )
EMLABCPP_GENERATE_DECOMPOSE( 13, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12 )
EMLABCPP_GENERATE_DECOMPOSE( 14, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13 )
EMLABCPP_GENERATE_DECOMPOSE( 15, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14 )
EMLABCPP_GENERATE_DECOMPOSE(
    16,
    a0,
    a1,
    a2,
    a3,
    a4,
    a5,
    a6,
    a7,
    a8,
    a9,
    a10,
    a11,
    a12,
    a13,
    a14,
    a15 )

template < typename T >
using decomposed_type = decltype( decompose( std::declval< T >() ) );

}  // namespace emlabcpp
