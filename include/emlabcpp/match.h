#include "emlabcpp/visit.h"

#pragma once

namespace emlabcpp
{

template < typename... Callables >
struct matcher : Callables...
{
        static constexpr bool is_nothrow_move_constructible =
            ( std::is_nothrow_move_constructible_v< Callables > && ... );

        matcher( const matcher& )                                      = default;
        matcher( matcher&& ) noexcept( is_nothrow_move_constructible ) = default;

        static constexpr bool is_nothrow_assignable =
            ( std::is_nothrow_move_assignable_v< Callables > && ... );

        matcher& operator=( const matcher& )                              = default;
        matcher& operator=( matcher&& ) noexcept( is_nothrow_assignable ) = default;

        template < typename... Ts >
        explicit matcher( Ts&&... ts )
          : Callables{ std::forward< Ts >( ts ) }...
        {
        }

        using Callables::operator()...;

        ~matcher() = default;
};

template < typename... Callables >
matcher( Callables&&... ) -> matcher< std::decay_t< Callables >... >;

template < typename Variant, typename... Callables >
auto match( Variant&& var, Callables&&... cals )
{
        return emlabcpp::visit(
            matcher< std::decay_t< Callables >... >( std::forward< Callables >( cals )... ),
            std::forward< Variant >( var ) );
}

template < typename Variant, typename... Callables >
auto apply_on_match( Variant&& var, Callables&&... cals )
{
        return apply_on_visit(
            matcher< std::decay_t< Callables >... >( std::forward< Callables >( cals )... ),
            std::forward< Variant >( var ) );
}

}  // namespace emlabcpp
