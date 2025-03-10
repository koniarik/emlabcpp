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

#include "emlabcpp/types.h"

#include <tuple>

namespace emlabcpp::impl
{

template < typename... Args, typename UnaryPredicate, std::size_t... Idx >
[[nodiscard]] constexpr std::size_t
find_if_impl( const std::tuple< Args... >& t, UnaryPredicate&& f, std::index_sequence< Idx... > )
{
        std::size_t res = sizeof...( Args );
        auto        ff  = [&]( const auto& item, const std::size_t i ) {
                if ( f( item ) ) {
                        res = i;
                        return true;
                }
                return false;
        };

        ( ff( std::get< Idx >( t ), Idx ) || ... );

        return res;
}

template <
    typename T,
    std::size_t     N,
    range_container Container,
    typename UnaryCallable,
    std::size_t... Is >
[[nodiscard]] std::array< T, N >
map_f_to_a_impl( Container&& cont, UnaryCallable&& f, std::integer_sequence< std::size_t, Is... > )
{

        auto iter    = cont.begin();
        auto process = [&]( auto ) {
                if constexpr ( std::is_reference_v< Container > )
                        return f( *iter++ );
                else
                        return f( std::move( *iter++ ) );
        };

        /// https://en.cppreference.com/w/cpp/language/eval_order
        /// based on standard the order of process(i) calls is defined only in case we are using
        /// constructor initializer with {} brackets. Otherwise it can be any order ...
        return std::array< T, N >{ process( Is )... };
}

template <
    typename T,
    std::size_t        N,
    gettable_container Container,
    typename UnaryCallable,
    std::size_t... Is >
requires( !range_container< Container > )
[[nodiscard]] std::array< T, N > map_f_to_a_impl(
    Container&&          cont,
    const UnaryCallable& f,
    std::integer_sequence< std::size_t, Is... > )
{
        auto process = [&cont, &f]< std::size_t i >() {
                return f( std::get< i >( std::forward< Container >( cont ) ) );
        };

        /// viz. second map_f_to_a_impl
        return std::array< T, N > { process.template operator()< Is >()... };
}

template < typename >
struct map_f_collector;

template < with_push_back T >
struct map_f_collector< T >
{
        void collect( T& res, typename T::value_type val ) const
        {
                res.push_back( std::move( val ) );
        }
};

template < typename T >
requires requires( T a, typename T::value_type b ) { a.insert( b ); }
struct map_f_collector< T >
{
        void collect( T& res, typename T::value_type val ) const
        {
                res.insert( std::move( val ) );
        }
};

template < typename T, std::size_t N >
struct map_f_collector< std::array< T, N > >
{
        std::size_t i = 0;

        void collect( std::array< T, N >& res, T val )
        {
                res[i] = std::move( val );
                i += 1;
        }
};

template < typename T >
concept map_f_collectable = requires( T item, typename T::value_type val ) {
        map_f_collector< T >{}.collect( item, std::move( val ) );
};

template < std::size_t I, typename T >
constexpr auto get_ith_item_from_arrays( T& arr, auto&... arrays )
{
        constexpr std::size_t first_size = std::tuple_size_v< std::decay_t< T > >;
        if constexpr ( I >= first_size )
                return get_ith_item_from_arrays< I - first_size >( arrays... );
        else
                return arr[I];
}

#define EMLABCPP_INDEX_MAX 32
#define EMLABCPP_INDEX_EXPAND( F ) \
        F( 0 )                     \
        F( 1 )                     \
        F( 2 )                     \
        F( 3 )                     \
        F( 4 )                     \
        F( 5 )                     \
        F( 6 )                     \
        F( 7 )                     \
        F( 8 )                     \
        F( 9 )                     \
        F( 10 )                    \
        F( 11 )                    \
        F( 12 )                    \
        F( 13 )                    \
        F( 14 )                    \
        F( 15 )                    \
        F( 16 )                    \
        F( 17 )                    \
        F( 18 )                    \
        F( 19 )                    \
        F( 20 )                    \
        F( 21 )                    \
        F( 22 )                    \
        F( 23 )                    \
        F( 24 )                    \
        F( 25 )                    \
        F( 26 )                    \
        F( 27 )                    \
        F( 28 )                    \
        F( 29 )                    \
        F( 30 )                    \
        F( 31 )

#define EMLABCPP_INDEX_SWITCH_CASE( x ) \
        case x:                         \
                if constexpr ( x < N )  \
                        return std::forward< F >( f ).template operator()< Off + x >();

/// Executes `f<i>` if provided `i` matches
template < std::size_t Off, std::size_t N, typename F >
constexpr decltype( auto ) index_switch( std::size_t i, F&& f )
{
        switch ( i ) {
                EMLABCPP_INDEX_EXPAND( EMLABCPP_INDEX_SWITCH_CASE )
        default:
                break;
        }
        if constexpr ( N > EMLABCPP_INDEX_MAX )
                return index_switch< EMLABCPP_INDEX_MAX, N - EMLABCPP_INDEX_MAX >(
                    i - EMLABCPP_INDEX_MAX, std::forward< F >( f ) );

        abort();
}

#define EMLABCPP_INDEX_SEQ( x ) \
        if constexpr ( x < N )  \
                f.template operator()< Off + x >();

/// Executes `f<i>` for i going from Off to N
template < std::size_t Off, std::size_t N, typename F >
constexpr void index_seq( F&& f )
{
        EMLABCPP_INDEX_EXPAND( EMLABCPP_INDEX_SEQ )
        if constexpr ( N > EMLABCPP_INDEX_MAX )
                index_seq< EMLABCPP_INDEX_MAX, N - EMLABCPP_INDEX_MAX >( std::forward< F >( f ) );
}

#define EMLABCPP_INDEX_UNTIL( x )                         \
        if constexpr ( x < N )                            \
                if ( f.template operator()< Off + x >() ) \
                        return true;

/// Executes `f<i>` until some is true
template < std::size_t Off, std::size_t N, typename F >
constexpr bool index_until( F&& f )
{
        EMLABCPP_INDEX_EXPAND( EMLABCPP_INDEX_UNTIL )
        if constexpr ( N > EMLABCPP_INDEX_MAX )
                return index_until< EMLABCPP_INDEX_MAX, N - EMLABCPP_INDEX_MAX >(
                    std::forward< F >( f ) );
        else
                return false;
}

}  // namespace emlabcpp::impl
