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
                if constexpr ( std::is_reference_v< Container > ) {
                        return f( *iter++ );
                } else {
                        return f( std::move( *iter++ ) );
                }
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
        return std::array< T, N >{ process.template operator()< Is >()... };
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

}  // namespace emlabcpp::impl
