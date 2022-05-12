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
#include "emlabcpp/iterators/numeric.h"
#include "emlabcpp/types.h"
#include "emlabcpp/view.h"

#include <tuple>

#pragma once

namespace emlabcpp
{
template < typename... >
class zip_iterator;
}

template < typename... Iterators >
struct std::iterator_traits< emlabcpp::zip_iterator< Iterators... > >
{
        using value_type = std::tuple< typename std::iterator_traits< Iterators >::reference... >;
        using difference_type   = std::ptrdiff_t;
        using pointer           = void;
        using reference         = value_type;
        using iterator_category = std::bidirectional_iterator_tag;
};

namespace emlabcpp
{

/// zip_ierator iterates over a group of iterators, where value is a tuple of references to value
/// for each iterator.
///
/// The design expects that all ranges of iterators are of same size.
///
template < typename... Iterators >
class zip_iterator
{
        std::tuple< Iterators... > iters_;

public:
        constexpr zip_iterator( Iterators... iters )
          : iters_( std::move( iters )... )
        {
        }

        /// Increases each iterator
        constexpr zip_iterator operator++()
        {
                std::apply(
                    []( auto&&... it ) {  //
                            ( ++it, ... );
                    },
                    iters_ );

                return *this;
        }

        /// Decreases each iterator
        constexpr zip_iterator operator--()
        {
                std::apply(
                    []( auto&&... it ) {  //
                            ( ++it, ... );
                    },
                    iters_ );

                return *this;
        }

        constexpr zip_iterator& operator+=( std::ptrdiff_t m )
        {
                for_each( iters_, [&]( auto& iter ) {
                        iter += m;
                } );
                return *this;
        }

        constexpr std::ptrdiff_t operator-( const zip_iterator< Iterators... >& other ) const
        {
                return std::get< 0 >( iters_ ) - std::get< 0 >( other.iters_ );
        }

        /// Dereference of each iterator, returns tuple of references to the
        /// operator* of iterators.
        constexpr auto operator*()
        {
                return std::apply(
                    []( auto&&... it ) {  //
                            return std::forward_as_tuple( ( *it )... );
                    },
                    iters_ );
        }

        /// Two zip iterators are equal if all of their iterators are equal
        constexpr bool operator==( const zip_iterator< Iterators... >& other ) const
        {
                return equals( other, std::index_sequence_for< Iterators... >{} );
        }

private:
        template < typename std::size_t... Idx >
        [[nodiscard]] constexpr bool
        equals( const zip_iterator< Iterators... >& other, std::index_sequence< Idx... > ) const
        {
                return ( ( std::get< Idx >( iters_ ) == std::get< Idx >( other.iters_ ) ) || ... );
        }
};

template < typename... Iterators >
constexpr zip_iterator< Iterators... >
operator+( zip_iterator< Iterators... > lh, std::ptrdiff_t m )
{
        lh += m;
        return lh;
}

template < typename... Iterators >
constexpr bool
operator!=( const zip_iterator< Iterators... >& lh, const zip_iterator< Iterators... >& rh )
{
        return !( lh == rh );
}

/// Creates a view of zip iterators for specified containers.
///
/// Beware that the function does not check that containers have same size of
/// ranges. If the size differs, increments of begin iterator will never be same
/// as end iterator.
//
template < range_container... Ts >
inline auto zip( Ts&&... cont )
{
        return view( zip_iterator( std::begin( cont )... ), zip_iterator( std::end( cont )... ) );
}

template < typename Container >
inline auto enumerate( Container&& cont )
{
        return zip( range( cont.size() ), cont );
}

template < typename TuplesTuple, std::size_t... ItemIndexes, std::size_t... TupleIndexes >
inline auto tuple_zip_impl(
    TuplesTuple&& tpls,
    std::index_sequence< ItemIndexes... >,
    std::index_sequence< TupleIndexes... > )
{
        auto f = [&]< typename Index >( Index ) {
                return std::make_tuple(
                    std::get< Index::value >( std::get< TupleIndexes >( tpls ) )... );
        };

        return std::make_tuple( f( std::integral_constant< std::size_t, ItemIndexes >{} )... );
}

/// Zips a set of tuples of same size into a new tuple.
///
/// zip(tuple<A,B>(), tuple<C,D>()) -> tuple<tuple<A,C>, <tuple<B,d>>;
///
template <
    typename Tuple,
    typename... Tuples,
    std::enable_if_t< is_std_tuple_v< Tuple > >*                                         = nullptr,
    std::enable_if_t< std::conjunction_v< is_std_tuple< std::decay_t< Tuples > >... > >* = nullptr >
inline auto zip( Tuple&& frst, Tuples&&... tpls )
{
        static_assert(
            ( (static_size_v< Tuple > == static_size_v< Tuples >) &&... ),
            "All tuples has to be of same size in zip" );
        return tuple_zip_impl(
            std::make_tuple( frst, tpls... ),
            std::make_index_sequence< static_size_v< Tuple > >{},
            std::make_index_sequence< sizeof...( Tuples ) + 1 >{} );
}

}  // namespace emlabcpp
