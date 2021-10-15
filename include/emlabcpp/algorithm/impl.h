#include "emlabcpp/types.h"

#include <tuple>

#pragma once

namespace emlabcpp::impl
{

template < typename... Args, typename UnaryFunction, std::size_t... Idx >
[[nodiscard]] constexpr std::size_t
find_if_impl( const std::tuple< Args... >& t, UnaryFunction&& f, std::index_sequence< Idx... > )
{
        std::size_t res = sizeof...( Args );
        auto        ff  = [&]( const auto& item, std::size_t i ) {
                if ( f( item ) ) {
                        res = i;
                        return true;
                }
                return false;
        };

        ( ff( std::get< Idx >( t ), Idx ) || ... );

        return res;
}

template < typename T, std::size_t N, typename Container, typename UnaryFunction, std::size_t... Is >
[[nodiscard]] inline std::array< T, N >
map_f_to_a_impl( Container&& cont, UnaryFunction&& f, std::integer_sequence< std::size_t, Is... > )
{

        auto iter    = cont.begin();
        auto process = [&]( auto ) {
                if constexpr ( std::is_reference_v< Container > ) {
                        return f( *iter++ );
                } else {
                        return f( std::move( *iter++ ) );
                }
        };

        // https://en.cppreference.com/w/cpp/language/eval_order
        // based on standard the order of process(i) calls is defined only in case we are using
        // constructor initializer with {} brackets. Otherwise it can be any order ...
        return std::array< T, N >{ process( Is )... };
}

template < typename >
struct map_f_collector;

template < typename T >
requires requires( T a, typename T::value_type b )
{
        a.push_back( b );
}
struct map_f_collector< T >
{
        void collect( T& res, typename T::value_type val )
        {
                res.push_back( std::move( val ) );
        }
};

template < typename T >
requires requires( T a, typename T::value_type b )
{
        a.insert( b );
}
struct map_f_collector< T >
{
        void collect( T& res, typename T::value_type val )
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
concept map_f_collectable = requires( T item, typename T::value_type val )
{
        map_f_collector< T >{}.collect( item, val );
};

}  // namespace emlabcpp::impl
