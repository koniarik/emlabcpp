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

template < ostreamlike Stream, auto ID >
auto& operator<<( Stream& os, tag< ID > )
{
        return os << ID;
}

/// ------------------------------------------------------------------------------------------------
//// central function for returning name of type that can demangle if necessary

template < typename T >
std::string pretty_type_name()
{
        std::string res;
#ifdef EMLABCPP_USE_DEMANGLING
        int   tmp   = 0;
        char* dname = abi::__cxa_demangle( typeid( T ).name(), nullptr, nullptr, &tmp );
        res         = dname;
        // NOLINTNEXTLINE
        free( dname );
#elif EMLABCPP_USE_TYPEID
        res = typeid( T ).name();
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

template < typename Signature >
struct signature_of;

template < typename ReturnType, typename Class, typename... Args >
struct signature_of< ReturnType ( Class::* )( Args... ) >
{
        using return_type = ReturnType;
        using class_type  = Class;
        using args_type   = std::tuple< Args... >;
};

template < typename ReturnType, typename... Args >
struct signature_of< ReturnType ( Args... ) >
{
        using return_type = ReturnType;
        using args_type   = std::tuple< Args... >;
};

}  // namespace emlabcpp
