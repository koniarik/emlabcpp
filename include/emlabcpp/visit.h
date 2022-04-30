#include <tuple>
#include <variant>

#pragma once

namespace emlabcpp
{
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

template < typename Visitor, typename Variant >
decltype( auto ) visit( Visitor&& vis, Variant&& var )
{
        return detail::linear_visit_impl< std::variant_size_v< std::decay_t< Variant > > - 1 >(
            std::forward< Variant >( var ), vis );
}

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

}  // namespace emlabcpp
