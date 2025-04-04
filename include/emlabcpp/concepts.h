/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
///

#pragma once

#include "./types/base.h"

#include <concepts>
#include <iterator>
#include <variant>

namespace emlabcpp
{

/// ------------------------------------------------------------------------------------------------
/// arithmetic related concepts

template < typename T >
concept additive_operators = requires( T a, T b ) {
        { a + b } -> std::convertible_to< T >;
        { a - b } -> std::convertible_to< T >;
};

template < typename T >
concept multiplicative_operators = requires( T a, T b ) {
        { a / b } -> std::convertible_to< T >;
        { a* b } -> std::convertible_to< T >;
};

template < typename T >
concept arithmetic_operators = additive_operators< T > && multiplicative_operators< T >;

template < typename T >
concept arithmetic_assignment = requires( T a, T b ) {
        { a += b };
        { a -= b };
        { a /= b };
        { a *= b };
};

template < typename T >
concept arithmetic_like = arithmetic_operators< T > && arithmetic_assignment< T >;

template < typename T >
concept arithmetic = std::integral< T > || std::floating_point< T >;

/// ------------------------------------------------------------------------------------------------
/// container related concepts

template < typename T >
concept gettable_container = requires( T a ) {
        { std::tuple_size< std::decay_t< T > >::value } -> std::convertible_to< std::size_t >;
};

/// so, std::ranges::range is meh because it expects return of begin() being input_output_iterator,
/// which has to be def.constructible
template < typename T >
concept range_container =
    ( (
          requires( T a ) { begin( a ); } || requires( T a ) { std::begin( a ); } ) &&
      (
          requires( T a ) { end( a ); } || requires( T a ) { std::end( a ); } ) ) ||
    std::is_bounded_array_v< T >;

template < typename T >
concept data_container =
    (
        requires( T a ) { data( a ); } || requires( T a ) { std::data( a ); } ) &&
    (
        requires( T a ) { size( a ); } || requires( T a ) { std::size( a ); } );

template < typename T >
concept container = range_container< T > || gettable_container< T > || data_container< T >;

template < typename T >
concept referenceable_container =
    is_view< T >::value || ( range_container< T > && !std::is_rvalue_reference_v< T > );

template < typename T, typename ValueType >
concept range_container_with =
    range_container< T > && std::same_as< typename T::value_type, ValueType >;

template < typename T, typename Iterator >
concept range_container_with_iter =
    range_container< T > && std::convertible_to< iterator_of_t< T >, Iterator >;

template < typename T, typename ValueType >
concept data_container_with =
    data_container< T > && std::same_as< typename T::value_type, ValueType >;

template < typename T, typename DataIterator >
concept data_container_with_iter =
    data_container< T > && std::convertible_to< data_iterator_of_t< T >, DataIterator >;

template < typename T >
concept static_sized = requires( T a ) {
        { std::tuple_size< std::decay_t< T > >::value } -> std::convertible_to< std::size_t >;
};

/// ------------------------------------------------------------------------------------------------
/// invocable related concepts

template < typename UnaryCallable, typename Container >
concept container_invocable =
    requires( Container cont, UnaryCallable f ) { f( *cont.begin() ); } ||
    requires( Container cont ) { std::tuple_size< std::decay_t< Container > >::value == 0; } ||
    requires( Container cont, UnaryCallable f ) {
            /// this has to come after the size check, as gcc 10.2 will faill to compile the code
            /// using this concept otherwise. If container is std::tuple<> and this check comes
            /// before the size one, it fails on std::get<0> being not compailable.
            f( std::get< 0 >( cont ) );
    };

template < typename UnaryCallable, typename ReturnValue, typename... Args >
concept invocable_returning = requires( UnaryCallable f, Args... args ) {
        { f( args... ) } -> std::same_as< ReturnValue >;
};

/// ------------------------------------------------------------------------------------------------
/// stream related concepts

namespace detail
{
        template < typename Stream, typename T >
        concept directly_streamable_for = requires( Stream os, T val ) { os.operator<<( val ); };
}  // namespace detail

template < typename T >
concept ostreamlike = !std::is_array_v< T > && requires( T val ) {
        requires detail::directly_streamable_for< T, uint8_t >;
        requires detail::directly_streamable_for< T, uint16_t >;
        requires detail::directly_streamable_for< T, uint32_t >;
        requires detail::directly_streamable_for< T, int8_t >;
        requires detail::directly_streamable_for< T, int16_t >;
        requires detail::directly_streamable_for< T, int32_t >;
        requires detail::directly_streamable_for< T, float >;
        requires detail::directly_streamable_for< T, double >;
        requires detail::directly_streamable_for< T, bool >;
        requires detail::directly_streamable_for< T, void const* >;
        requires detail::directly_streamable_for< T, std::nullptr_t >;
};

template < typename T >
concept ostreamable = requires( std::ostream& os, T item ) {
        { os << item } -> std::convertible_to< std::ostream& >;
};

/// ------------------------------------------------------------------------------------------------

/// Thanks for the solution goes to PJBoy@libera
template < typename T, typename Variant >
concept alternative_of = []< typename... Ts >( std::variant< Ts... >* ) {
        return ( std::same_as< T, Ts > || ... );
}( static_cast< Variant* >( nullptr ) );

template < typename T, typename Tuple >
concept element_of = []< typename... Ts >( std::tuple< Ts... >* ) {
        return ( std::same_as< T, Ts > || ... );
}( static_cast< Tuple* >( nullptr ) );

template < typename T >
concept with_value_type = requires { typename T::value_type; };

template < typename T, typename Signature >
concept with_signature = std::same_as< typename signature_of< T >::signature, Signature >;

template < typename T >
concept with_push_back =
    requires( T a, typename T::value_type b ) { a.push_back( std::move( b ) ); };

template < typename T, typename U >
concept some = std::same_as< std::remove_cv_t< T >, U >;

}  // namespace emlabcpp
