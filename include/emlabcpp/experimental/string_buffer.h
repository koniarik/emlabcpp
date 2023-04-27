
#include "emlabcpp/experimental/pretty_printer.h"

#include <algorithm>
#include <array>

#pragma once

namespace emlabcpp
{

template < std::size_t N >
struct string_buffer : std::array< char, N >
{
        using base_type = std::array< char, N >;

        constexpr string_buffer()
          : base_type{}
        {
                std::fill_n(this->data(), N, '\0');
        }

        constexpr string_buffer( std::string_view sv )
          : string_buffer()
        {
                std::copy_n( sv.begin(), std::min( sv.size(), N - 1 ), this->begin() );
        }

        template < std::size_t M >
        constexpr string_buffer( const char ( &msg )[M] )
          : string_buffer()
        {
                static_assert( M < N );
                std::copy_n( msg, M, this->begin() );
        }

        operator std::string_view() const
        {
                return std::string_view( this->data(), strlen(this->data()) );
        }
};

template < std::size_t N >
std::ostream& operator<<( std::ostream& os, const string_buffer< N >& sb )
{
        return os << std::string_view( sb );
}

template < std::size_t N >
struct pretty_printer< string_buffer< N > >
{
        template < typename W >
        static void print( W&& w, const string_buffer< N >& buff )
        {
                w( std::string_view( buff ) );
        }
};

}  // namespace emlabcpp
