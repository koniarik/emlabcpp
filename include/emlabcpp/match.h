#include "emlabcpp/visit.h"

#pragma once

namespace emlabcpp
{

template < typename... Callables >
struct matcher : Callables...
{
        matcher( const matcher& )     = default;
        matcher( matcher&& ) noexcept = default;

        matcher& operator=( const matcher& ) = default;
        matcher& operator=( matcher&& ) = default;

        template < typename... Ts >
        explicit matcher( Ts&&... ts )
          : Callables{ std::forward< Ts >( ts ) }...
        {
        }

        using Callables::operator()...;

        ~matcher() = default;
};

template < typename Variant, typename... Callables >
auto match( Variant&& var, Callables&&... cals )
{
        return visit(
            matcher< std::decay_t< Callables >... >( std::forward< Callables >( cals )... ), var );
}

template < typename Variant, typename... Callables >
auto apply_on_match( Variant&& var, Callables&&... cals )
{
        return apply_on_visit(
            matcher< std::decay_t< Callables >... >( std::forward< Callables >( cals )... ), var );
}

}  // namespace emlabcpp
