#include "emlabcpp/concepts.h"

#include <array>

#pragma once

namespace emlabcpp
{

template < std::size_t N >
class protocol_message
{

public:
        using value_type     = uint8_t;
        using iterator       = uint8_t*;
        using const_iterator = const uint8_t*;

        static constexpr std::size_t max_size = N;

        static std::optional< protocol_message > make( const range_container auto& cont )
        {
                if ( cont.size() > N ) {
                        return {};
                }
                return { protocol_message( cont.begin(), cont.end() ) };
        }

        protocol_message() = default;

        template < std::size_t M >
        explicit protocol_message( protocol_message< M > other ) noexcept
          : protocol_message( other.begin(), other.end() )
        {
                static_assert( M <= N );
        }

        template < std::size_t M >
        explicit protocol_message( const std::array< uint8_t, M >& inpt ) noexcept
          : protocol_message( inpt.begin(), inpt.begin() + M )
        {
                static_assert( M <= N );
        }

        [[nodiscard]] std::size_t size() const
        {
                return used_;
        }

        [[nodiscard]] uint8_t front() const
        {
                return data_.front();
        }

        [[nodiscard]] uint8_t& front()
        {
                return data_.front();
        }

        [[nodiscard]] uint8_t back() const
        {
                return data_[used_ - 1];
        }

        [[nodiscard]] uint8_t& back()
        {
                return data_.front();
        }

        [[nodiscard]] const_iterator begin() const
        {
                return data_.begin();
        }

        [[nodiscard]] iterator begin()
        {
                return data_.begin();
        }

        [[nodiscard]] const_iterator end() const
        {
                return data_.begin() + used_;
        }

        [[nodiscard]] iterator end()
        {
                return data_.begin() + used_;
        }

        uint8_t operator[]( std::size_t i ) const
        {
                return data_[i];
        }

        uint8_t& operator[]( std::size_t i )
        {
                return data_[i];
        }

        friend auto operator==( const protocol_message& lh, const protocol_message& rh )
        {
                return view_n( lh.data_.begin(), lh.used_ ) == view_n( rh.data_.begin(), rh.used_ );
        }

private:
        std::array< uint8_t, N > data_ = { 0 };
        std::size_t              used_ = { 0 };

protected:
        template < typename Iterator >
        protocol_message( Iterator begin, Iterator end ) noexcept
          : used_( static_cast< std::size_t >( std::distance( begin, end ) ) )
        {
                std::copy( begin, end, data_.begin() );
        }
};

template < std::size_t N >
class protocol_sizeless_message : public protocol_message< N >
{
public:
        using protocol_message< N >::protocol_message;

        static std::optional< protocol_sizeless_message > make( const range_container auto& cont )
        {
                if ( cont.size() > N ) {
                        return {};
                }
                return { protocol_sizeless_message( cont.begin(), cont.end() ) };
        }
};

}  // namespace emlabcpp
