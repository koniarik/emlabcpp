#pragma once

#include <concepts>

namespace emlabcpp
{

template < typename Derived, typename Enum, Enum SuccessValue >
struct [[nodiscard]] status
{
        using enum_type = Enum;

        status( enum_type s ) noexcept
          : s_( s )
        {
        }

        status( status const& ) noexcept            = default;
        status( status&& ) noexcept                 = default;
        status& operator=( status const& ) noexcept = default;
        status& operator=( status&& ) noexcept      = default;

        friend auto operator<=>( status const& lhs, status const& rhs ) noexcept = default;

        friend auto operator<=>( status const& lhs, Derived const& rhs ) noexcept
        {
                return lhs.s_ <=> rhs;
        }

        friend bool operator==( status const& lhs, Derived const& rhs ) noexcept
        {
                return lhs.s_ == static_cast< status const& >( rhs ).s_;
        }

private:
        enum_type s_;
};

struct [[nodiscard]] success_type
{
};

constexpr success_type const SUCCESS;

struct [[nodiscard]] failure_type
{
};

constexpr failure_type const FAILURE;

struct [[nodiscard]] error_type
{
};

constexpr error_type const ERROR;

}  // namespace emlabcpp