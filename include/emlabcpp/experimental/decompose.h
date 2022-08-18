#include <tuple>

#pragma once

namespace emlabcpp
{

// thanks to the great presentation: https://www.youtube.com/watch?v=abdeAew3gmQ (Antony Polukhin)
namespace detail
{
        template < std::size_t >
        struct decompose_anything
        {
                template < typename T >
                operator T();
        };

        template < class T, std::size_t I0, std::size_t... I >
        constexpr auto decompose_count( int& out, std::index_sequence< I0, I... > )
            -> std::add_pointer_t<
                decltype( T{ decompose_anything< I0 >{}, decompose_anything< I >{}... } ) >
        {
                out = sizeof...( I ) + 1;
                return nullptr;
        }
        template < class T, std::size_t... I >
        constexpr void* decompose_count( int& out, std::index_sequence< I... > )
        {
                if constexpr ( sizeof...( I ) > 0 ) {
                        decompose_count< T >(
                            out, std::make_index_sequence< sizeof...( I ) - 1 >{} );
                } else {
                        out = -1;
                }
                return nullptr;
        }

        template < typename T, std::size_t N >
        constexpr bool decompose_has_size()
        {
                int c = -2;
                decompose_count< std::decay_t< T > >( c, std::make_index_sequence< 16 >{} );
                return c == N;
        }
}  // namespace detail

#define EMLABCPP_GENERATE_DECOMPOSE( ID, ... )                                 \
        template < typename T >                                                \
        concept decomposable_##ID = detail::decompose_has_size< T, ( ID ) >(); \
                                                                               \
        template < decomposable_##ID T >                                       \
        constexpr auto decompose( T&& item )                                   \
        {                                                                      \
                if constexpr ( std::is_rvalue_reference_v< T > ) {             \
                        auto&& [__VA_ARGS__] = std::move( item );              \
                        return std::make_tuple( __VA_ARGS__ );                 \
                } else {                                                       \
                        auto& [__VA_ARGS__] = item;                            \
                        return std::tie( __VA_ARGS__ );                        \
                }                                                              \
        }

template < typename T, typename Tuple >
constexpr T compose( Tuple tpl )
{
        return std::make_from_tuple< T >( std::move( tpl ) );
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

}  // namespace emlabcpp
