#pragma once

#include "emlabcpp/experimental/pretty_printer.h"

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

        [[nodiscard]] constexpr std::size_t size() const
        {
                return strlen( this->data() );
        }
};

template < std::size_t N >
std::ostream& operator<<( std::ostream& os, string_buffer< N > const& sb )
{
        return os << std::string_view( sb );
}

template < std::size_t N >
struct pretty_printer< string_buffer< N > >
{
        template < typename W >
        static void print( W&& w, string_buffer< N > const& buff )
        {
                w( std::string_view{ buff } );
        }
};

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
