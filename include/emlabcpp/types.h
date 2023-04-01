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

#include "emlabcpp/concepts.h"
#include "emlabcpp/types/base.h"

#include <string>

#ifdef EMLABCPP_USE_DEMANGLING
#include <cxxabi.h>
#endif

#pragma once

namespace emlabcpp
{

/// ------------------------------------------------------------------------------------------------
/// has_static_size<T>::value is true in case type T have size deduceable at compile time

template < typename T >
constexpr bool has_static_size_v = static_sized< T >;

/// ------------------------------------------------------------------------------------------------
/// mapped<T,F>::type is type returned by instance of F::operator() when applied on items from
/// instance of T. It can differentiate between tuples or containers

template < typename Container, typename UnaryCallable >
struct mapped;

template < gettable_container Container, typename UnaryCallable >
requires( !range_container< Container > ) struct mapped< Container, UnaryCallable >
{
        using type = decltype( std::declval< UnaryCallable >()(
            std::get< 0 >( std::declval< Container >() ) ) );
};

template < range_container Container, typename UnaryCallable >
struct mapped< Container, UnaryCallable >
{
        using type = decltype( std::declval< UnaryCallable >()(
            *std::begin( std::declval< Container >() ) ) );
};

template < typename Container, typename UnaryCallable >
using mapped_t = typename mapped< Container, UnaryCallable >::type;

/// ------------------------------------------------------------------------------------------------
//// tag<V> type can be used for tagging f-calls for function overloading
template < auto V >
struct tag
{
        using value_type = decltype( V );

        static constexpr value_type value = V;

        friend constexpr auto operator<=>( const tag&, const tag& ) = default;
};

#ifdef EMLABCPP_USE_OSTREAM
template < auto ID >
std::ostream& operator<<( std::ostream& os, tag< ID > )
{
        return os << ID;
}
#endif

/// ------------------------------------------------------------------------------------------------
//// central function for returning name of type that can demangle if necessary

template < typename T >
auto pretty_type_name()
{
#ifdef EMLABCPP_USE_DEMANGLING
        std::string res;
        int         tmp   = 0;
        char* const dname = abi::__cxa_demangle( typeid( T ).name(), nullptr, nullptr, &tmp );
        res               = dname;
        // NOLINTNEXTLINE
        free( dname );
#elif defined EMLABCPP_USE_TYPEID
        std::string_view res = typeid( T ).name();
#else
        std::string_view res = "type names not supported";
#endif
        return res;
}

/// ------------------------------------------------------------------------------------------------

template < std::size_t >
struct select_utype;

template < std::size_t N >
requires( sizeof( uint8_t ) == N ) struct select_utype< N >
{
        using type = uint8_t;
};
template < std::size_t N >
requires( sizeof( uint16_t ) == N ) struct select_utype< N >
{
        using type = uint16_t;
};
template < std::size_t N >
requires( sizeof( uint32_t ) == N ) struct select_utype< N >
{
        using type = uint32_t;
};
template < std::size_t N >
requires( sizeof( uint64_t ) == N ) struct select_utype< N >
{
        using type = uint64_t;
};

template < std::size_t N >
using select_utype_t = typename select_utype< N >::type;

/// ------------------------------------------------------------------------------------------------

template < typename, template < typename > class >
struct type_map;

template < typename... Ts, template < typename > class Fun >
struct type_map< std::tuple< Ts... >, Fun >
{
        using type = std::tuple< Fun< Ts >... >;
};

template < typename T, template < typename > class Fun >
using type_map_t = typename type_map< T, Fun >::type;

/// ------------------------------------------------------------------------------------------------

template < typename T >
struct type_tag
{
        using type = T;
};

/// ------------------------------------------------------------------------------------------------

template < typename T, typename Variant >
struct index_of;

template < typename T, typename... Ts >
struct index_of< T, std::variant< Ts... > >
  : std::integral_constant<
        std::size_t,
        std::variant< type_tag< Ts >... >( type_tag< T >() ).index() >
{
        // got the tip for this from:
        // https://stackoverflow.com/questions/52303316/get-index-by-type-in-stdvariant
};

}  // namespace emlabcpp
