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
#include <tuple>
#include <variant>

#pragma once

namespace emlabcpp
{
namespace detail
{
        template < std::size_t N, typename Variant >
        decltype( auto ) linear_visit_impl( Variant&& var, auto& cb );
}  // namespace detail

/// Reimplementation of `std::visit`. This one trades worse complexity (linear) in favor of less
/// assembly generated.
template < typename Visitor, typename Variant >
decltype( auto ) visit( Visitor&& vis, Variant&& var )
{
        return detail::linear_visit_impl< std::variant_size_v< std::decay_t< Variant > > - 1 >(
            std::forward< Variant >( var ), vis );
}

/// Combines `visit` and `std::apply` into one step - provided variant is expanded with `visit` and
/// `apply` is called on the present alternative, items from `apply` are passed to calle to visitor
/// `vis`.
template < typename Visitor, typename Variant >
decltype( auto ) apply_on_visit( Visitor&& vis, Variant&& var )
{
        return emlabcpp::visit(
            [&]< typename Item >( Item&& item ) -> decltype( auto ) {
                    return std::apply(
                        [&]< typename... Vals >( Vals&&... vals ) -> decltype( auto ) {
                                return vis( std::forward< Vals >( vals )... );
                        },
                        std::forward< Item >( item ) );
            },
            std::forward< Variant >( var ) );
}
namespace detail
{
        template < std::size_t N, typename Variant >
        decltype( auto ) linear_visit_impl( Variant&& var, auto& cb )
        {
                if constexpr ( N == 0 ) {
                        return cb( std::get< 0 >( std::forward< Variant >( var ) ) );
                } else {
                        if ( var.index() == N ) {
                                return cb( std::get< N >( std::forward< Variant >( var ) ) );
                        } else {
                                return linear_visit_impl< N - 1 >(
                                    std::forward< Variant >( var ), cb );
                        }
                }
        }
}  // namespace detail

}  // namespace emlabcpp
