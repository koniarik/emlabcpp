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

#include "./visit.h"

namespace emlabcpp
{

template < typename... Callables >
struct matcher : Callables...
{
        matcher( matcher const& )     = default;
        matcher( matcher&& ) noexcept = default;

        matcher& operator=( matcher const& )     = default;
        matcher& operator=( matcher&& ) noexcept = default;

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
decltype( auto ) match( Variant&& var, Callables&&... cals )
{
        return emlabcpp::visit(
            matcher< std::decay_t< Callables >... >( std::forward< Callables >( cals )... ),
            std::forward< Variant >( var ) );
}

template < typename Variant, typename... Callables >
decltype( auto ) apply_on_match( Variant&& var, Callables&&... cals )
{
        return apply_on_visit(
            matcher< std::decay_t< Callables >... >( std::forward< Callables >( cals )... ),
            std::forward< Variant >( var ) );
}

}  // namespace emlabcpp
