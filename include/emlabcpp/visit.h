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

#include <tuple>
#include <variant>

namespace emlabcpp
{
namespace detail
{
        template < std::size_t N, typename Callable >
        decltype( auto ) linear_index_visit_impl( std::size_t index, Callable&& cb );
}  // namespace detail

template < typename Visitor, typename Variant >
decltype( auto ) visit_index( Visitor&& vis, Variant const& var )
{
        return detail::linear_index_visit_impl<
            std::variant_size_v< std::decay_t< Variant > > - 1 >(
            var.index(), std::forward< Visitor >( vis ) );
}

/// Reimplementation of `std::visit`. This one trades worse complexity (linear) in favor of less
/// assembly generated.
template < typename Visitor, typename Variant >
decltype( auto ) visit( Visitor&& vis, Variant&& var )
{
        return visit_index(
            [&vis, &var]< std::size_t i >() {
                    return vis( *std::get_if< i >( &var ) );
            },
            std::forward< Variant >( var ) );
}

/// Combines `visit` and `std::apply` into one step - provided variant is expanded with `visit` and
/// `apply` is called on the present alternative, items from `apply` are passed to calle to visitor
/// `vis`.
template < typename Visitor, typename Variant >
decltype( auto ) apply_on_visit( Visitor&& vis, Variant&& var )
{
        return emlabcpp::visit(
            [&vis]< typename Item >( Item&& item ) -> decltype( auto ) {
                    return std::apply(
                        [&vis]< typename... Vals >( Vals&&... vals ) -> decltype( auto ) {
                                return vis( std::forward< Vals >( vals )... );
                        },
                        std::forward< Item >( item ) );
            },
            std::forward< Variant >( var ) );
}

namespace detail
{
        template < std::size_t N, typename Callable >
        decltype( auto ) linear_index_visit_impl( std::size_t index, Callable&& cb )
        {
                if constexpr ( N == 0 ) {
                        return std::forward< Callable >( cb ).template operator()< 0 >();
                } else if ( index == N ) {
                        return std::forward< Callable >( cb ).template operator()< N >();
                } else {
                        return linear_index_visit_impl< N - 1 >(
                            index, std::forward< Callable >( cb ) );
                }
        }
}  // namespace detail

}  // namespace emlabcpp
