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

#include "./concepts.h"
#include "./static_storage.h"
#include "./view.h"

namespace emlabcpp
{

/// Data container for up to N elements
template < typename T, std::size_t N >
class static_vector
{

public:
        static constexpr std::size_t capacity = N;

        using value_type      = T;
        using size_type       = std::size_t;
        using reference       = T&;
        using const_reference = T const&;
        using iterator        = T*;
        using const_iterator  = T const*;

        static_vector() = default;

        static_vector( static_vector const& other )
        {
                copy_from( other );
        }

        static_vector( static_vector&& other ) noexcept
        {
                move_from( other );
                other.clear();
        }

        static_vector( std::size_t M, T const& item )
        {
                M = std::max( M, N );
                std::uninitialized_fill( begin(), begin() + M, item );
        }

        template < std::size_t M >
        requires( M <= N )
        explicit static_vector( std::array< T, M > data )
        {
                move_from( data );
        }

        static_vector& operator=( static_vector const& other )
        {
                if ( this == &other )
                        return *this;
                clear();
                copy_from( other );
                return *this;
        }

        static_vector& operator=( static_vector&& other ) noexcept
        {
                if ( this == &other )
                        return *this;
                clear();
                move_from( other );
                other.clear();
                return *this;
        }

        template < std::size_t M >
        requires( M <= N )
        static_vector& operator=( std::array< T, M > data )
        {
                clear();
                move_from( data );
                return *this;
        }

        void swap( static_vector& other ) noexcept
        {
                using std::swap;
                size_type const shared_n = std::min( size(), other.size() );

                for ( size_type i = 0; i < shared_n; ++i )
                        swap( storage_[i], other.storage_[i] );

                if ( size() > other.size() ) {
                        for ( size_type i = shared_n; i < size(); ++i )
                                other.emplace_back( std::move( storage_[i] ) );
                        while ( shared_n != size() )
                                pop_back();
                } else if ( size() < other.size() ) {
                        for ( size_type i = shared_n; i < other.size(); ++i )
                                emplace_back( std::move( other.storage_[i] ) );
                        while ( shared_n != other.size() )
                                other.pop_back();
                }
        }

        [[nodiscard]] T* data()
        {
                return storage_.data();
        }

        [[nodiscard]] T const* data() const
        {
                return storage_.data();
        }

        [[nodiscard]] iterator begin()
        {
                return data();
        }

        [[nodiscard]] iterator end()
        {
                return begin() + size_;
        }

        [[nodiscard]] const_iterator begin() const
        {
                return data();
        }

        [[nodiscard]] const_iterator end() const
        {
                return begin() + size_;
        }

        [[nodiscard]] reference front()
        {
                return storage_[0];
        }

        [[nodiscard]] const_reference front() const
        {
                return storage_[0];
        }

        void push_back( T item )
        {
                emplace_back( std::move( item ) );
        }

        template < typename... Args >
        T& emplace_back( Args&&... args )
        {
                T& ref = storage_.emplace_item( size_, std::forward< Args >( args )... );
                size_ += 1;
                return ref;
        }

        [[nodiscard]] T take_back()
        {
                T item = std::move( back() );
                pop_back();
                return item;
        }

        void pop_back()
        {
                storage_.delete_item( size_ - 1 );
                size_ -= 1;
        }

        [[nodiscard]] reference back()
        {
                return storage_[size_ - 1];
        }

        [[nodiscard]] const_reference back() const
        {
                return storage_[size_ - 1];
        }

        /// other methods

        [[nodiscard]] constexpr std::size_t max_size() const
        {
                return N;
        }

        [[nodiscard]] std::size_t size() const
        {
                return size_;
        }

        [[nodiscard]] bool empty() const
        {
                return size_ == 0;
        }

        [[nodiscard]] bool full() const
        {
                return size_ == N;
        }

        const_reference operator[]( size_type const i ) const
        {
                return storage_[i];
        }

        reference operator[]( size_type const i )
        {
                return storage_[i];
        }

        void clear()
        {
                purge();
        }

        ~static_vector()
        {
                purge();
        }

private:
        static_storage< T, N > storage_;
        size_type              size_ = 0;  /// count of items

        template < typename Container >
        void copy_from( Container const& cont )
        {
                size_ = std::size( cont );
                std::uninitialized_copy( std::begin( cont ), std::end( cont ), begin() );
        }

        template < typename Container >
        void move_from( Container& cont )
        {
                size_ = std::size( cont );
                std::uninitialized_move( std::begin( cont ), std::end( cont ), begin() );
        }

        /// Cleans entire buffer from items.
        void purge()
        {
                std::destroy_n( begin(), size_ );
                size_ = 0;
        }
};

template < typename T, std::size_t N >
[[nodiscard]] auto operator<=>( static_vector< T, N > const& lh, static_vector< T, N > const& rh )
{
        return std::lexicographical_compare_three_way(
            std::begin( lh ), std::end( lh ), std::begin( rh ), std::end( rh ) );
}

template < typename T, std::size_t N >
[[nodiscard]] bool operator==( static_vector< T, N > const& lh, static_vector< T, N > const& rh )
{
        auto size = std::size( lh );
        if ( size != std::size( rh ) )
                return false;

        for ( std::size_t i = 0; i < size; ++i )
                if ( lh[i] != rh[i] )
                        return false;
        return true;
}

template < typename T, std::size_t N >
[[nodiscard]] bool operator!=( static_vector< T, N > const& lh, static_vector< T, N > const& rh )
{
        return !( lh == rh );
}

template < typename T, std::size_t N >
void swap( static_vector< T, N > const& lh, static_vector< T, N > const& rh ) noexcept
{
        lh.swap( rh );
}

#ifdef EMLABCPP_USE_OSTREAM
/// Output operator for the view, uses comma to separate the items in the view.
template < typename T, std::size_t N >
std::ostream& operator<<( std::ostream& os, static_vector< T, N > const& vec )
{
        return os << view{ vec };
}
#endif

}  // namespace emlabcpp
