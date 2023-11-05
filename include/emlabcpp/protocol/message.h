///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
/// and associated documentation files (the "Software"), to deal in the Software without
/// restriction, including without limitation the rights to use, copy, modify, merge, publish,
/// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or
/// substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
/// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
/// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#pragma once

#include "emlabcpp/concepts.h"
#include "emlabcpp/view.h"

#ifdef EMLABCPP_USE_NLOHMANN_JSON
#include <nlohmann/json.hpp>
#endif

namespace emlabcpp::protocol
{

/// Protocol library has custom type that represents message, however this is just simple overaly
/// over std::array that remembers how many bytes are used.
template < std::size_t N >
class message
{
public:
        using value_type     = std::byte;
        using reference      = std::byte&;
        using pointer        = std::byte*;
        using const_pointer  = const std::byte*;
        using iterator       = std::byte*;
        using const_iterator = const std::byte*;
        using size_type      = std::size_t;

        static constexpr size_type capacity = N;

        constexpr message() = default;

        constexpr explicit message( const size_type n ) noexcept
          : used_( n )
        {
        }

        constexpr explicit message( const view< const_iterator >& v ) noexcept
          : used_( v.size() )
        {
                std::copy( v.begin(), v.end(), std::data( data_ ) );
        }

        template < size_type M >
        constexpr explicit message( const message< M >& other ) noexcept
          : message( view{ other } )
        {
                static_assert( M <= N );
        }

        template < size_type M >
        constexpr explicit message( const std::array< value_type, M >& inpt ) noexcept
          : message( view{ inpt } )
        {
                static_assert( M <= N );
        }

        template < std::convertible_to< uint8_t >... Ts >
        constexpr explicit message( Ts... inpt )
          : data_{ static_cast< value_type >( inpt )... }
          , used_( sizeof...( Ts ) )
        {
        }

        template < size_type M >
        constexpr message& operator=( const message< M >& other ) noexcept
        {
                used_ = other.size();
                std::copy( other.begin(), other.end(), std::data( data_ ) );
                return *this;
        }

        [[nodiscard]] constexpr const_pointer data() const noexcept
        {
                return &data_[0];
        }

        [[nodiscard]] constexpr pointer data() noexcept
        {
                return &data_[0];
        }

        [[nodiscard]] constexpr size_type size() const noexcept
        {
                return used_;
        }

        [[nodiscard]] constexpr value_type front() const noexcept
        {
                return data_[0];
        }

        [[nodiscard]] constexpr reference front() noexcept
        {
                return data_[0];
        }

        [[nodiscard]] constexpr value_type back() const noexcept
        {
                return data_[used_ - 1];
        }

        [[nodiscard]] constexpr reference back() noexcept
        {
                return data_[used_ - 1];
        }

        [[nodiscard]] constexpr const_iterator begin() const noexcept
        {
                return &data_[0];
        }

        [[nodiscard]] constexpr iterator begin() noexcept
        {
                return &data_[0];
        }

        [[nodiscard]] constexpr const_iterator end() const noexcept
        {
                return &data_[0] + used_;
        }

        [[nodiscard]] constexpr iterator end() noexcept
        {
                return &data_[0] + used_;
        }

        value_type operator[]( const size_type i ) const noexcept
        {
                return data_[i];
        }

        reference operator[]( const size_type i ) noexcept
        {
                return data_[i];
        }

        void resize( const size_type n ) noexcept
        {
                used_ = n;
        }

        friend auto operator==( const message& lh, const message& rh ) noexcept
        {
                return view_n( lh.begin(), lh.used_ ) == view_n( rh.begin(), rh.used_ );
        }

private:
        std::byte data_[N];
        size_type used_ = { 0 };
};

template < std::convertible_to< uint8_t >... Ts >
message( Ts... inpt ) -> message< sizeof...( Ts ) >;

/// Sizeless message is class that behaves in a same way as normal message, however it is
/// serialized differently. Protocol message stores how many bytes it's made of before the data
/// itself in the final message, sizless message does not and greedely tries to parse rest of the
/// buffer during the parsing process.
template < std::size_t N >
class sizeless_message : public message< N >
{
public:
        using message< N >::message;

        template < std::size_t M >
        explicit sizeless_message( const message< M >& other )
          : message< N >( other )
        {
                static_assert( M <= N );
        }
};

template < std::convertible_to< uint8_t >... Ts >
sizeless_message( Ts... inpt ) -> sizeless_message< sizeof...( Ts ) >;

namespace detail
{
        template < std::size_t N >
        constexpr bool message_derived_test( const message< N >& )
        {
                return true;
        }
}  // namespace detail

/// concept matches any type that is message or derives from it.
template < typename T >
concept message_derived = requires( T val ) { detail::message_derived_test( val ); };

#ifdef EMLABCPP_USE_NLOHMANN_JSON

template < std::size_t N >
void to_json( nlohmann::json& j, const message< N >& msg )
{
        j = nlohmann::json::array();
        for ( std::byte b : msg ) {
                j.push_back( b );
        }
}

template < std::size_t N >
void from_json( const nlohmann::json& j, message< N >& msg )
{
        if ( j.size() > N ) {
                throw std::exception{};  // TODO: fix this
        }

        std::vector< std::byte > tmp;
        for ( std::byte b : j ) {
                tmp.push_back( b );
        }

        msg = message< N >{ view< const std::byte* >( tmp ) };
}

#endif

}  // namespace emlabcpp::protocol
