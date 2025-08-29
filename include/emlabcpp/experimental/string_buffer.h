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
#pragma once

#include <algorithm>
#include <array>
#include <cstring>
#include <string_view>

#ifdef EMLABCPP_USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

namespace emlabcpp
{

template < std::size_t N >
struct string_buffer : std::array< char, N >
{
        static constexpr std::size_t capacity = N;
        using base_type                       = std::array< char, N >;

        constexpr string_buffer()
          : base_type{}
        {
                std::fill_n( this->data(), N, '\0' );
        }

        constexpr string_buffer( std::string_view sv )
          : string_buffer()
        {
                std::copy_n( sv.begin(), std::min( sv.size(), N - 1 ), this->begin() );
        }

        template < std::size_t M >
        constexpr string_buffer( char const ( &msg )[M] )
          : string_buffer()
        {
                static_assert( M < N );
                std::copy_n( msg, M, this->begin() );
        }

        template < std::size_t M >
        constexpr string_buffer( string_buffer< M > const& msg )
          : string_buffer()
        {
                static_assert( M < N );
                std::copy_n( msg.data(), M, this->begin() );
        }

        operator std::string_view() const
        {
                return std::string_view( this->data() );
        }

        std::string_view sv() const
        {
                return std::string_view( this->data() );
        }

        [[nodiscard]] constexpr std::size_t size() const
        {
                return strlen( this->data() );
        }

        template < std::size_t M >
        constexpr bool operator==( string_buffer< M > const& other ) const noexcept
        {
                return std::string_view{ *this } == std::string_view{ other };
        }

        constexpr bool operator==( std::string_view other ) const noexcept
        {
                return std::string_view{ *this } == other;
        }
};

namespace bits
{
        template < typename T >
        struct is_string_buffer
        {
                static constexpr bool value = false;
        };

        template < std::size_t N >
        struct is_string_buffer< string_buffer< N > >
        {
                static constexpr bool value = true;
        };

}  // namespace bits

template < typename T >
concept is_string_buffer_v = bits::is_string_buffer< T >::value;

template < std::size_t N >
std::ostream& operator<<( std::ostream& os, string_buffer< N > const& sb )
{
        return os << std::string_view( sb );
}

#ifdef EMLABCPP_USE_NLOHMANN_JSON

template < std::size_t N >
void to_json( nlohmann::json& j, string_buffer< N > const& buffer )
{
        j = std::string_view{ buffer };
}

template < std::size_t N >
void from_json( nlohmann::json const& j, string_buffer< N >& buffer )
{
        std::string const s = j;
        buffer              = string_buffer< N >( std::string_view{ s } );
}

#endif

}  // namespace emlabcpp
