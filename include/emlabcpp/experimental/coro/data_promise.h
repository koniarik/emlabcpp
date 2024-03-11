#pragma once

#include <utility>

namespace emlabcpp::coro
{

template < typename T >
struct data_promise
{
        template < typename U >
        void return_value( U&& val )
        {
                value = std::forward< U >( val );
        }

        T value;
};

template <>
struct data_promise< void >
{
        void return_void() const
        {
        }
};

}  // namespace emlabcpp::coro
