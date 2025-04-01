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

#include "./iterator.h"
#include "./static_storage.h"
#include "./view.h"

#include <limits>

namespace emlabcpp
{

template < typename T >
class static_circular_buffer_iterator;

/// Class implementing circular buffer of any type for up to N elements. This should work for
/// generic type T, not just simple types.
///
/// It is safe in "single consumer single producer" scenario between main loop and interrupts.
/// Because of that the behavior is as follows:
///  - on insertion, item is inserted and than index is advanced
///  - on removal, item is removed and than index is advanced
///
/// In case of copy or move operations, the buffer does not have to store the data internally in
/// same manner, the data are equivavlent only from the perspective of push/pop operations.
///
template < typename T, std::size_t N >
class static_circular_buffer
{

public:
        static constexpr std::size_t max_size = N;

        // XXX: derive this
        using index_type = int32_t;

        static_assert(
            N < std::numeric_limits< index_type >::max(),
            "static_circular_buffer: N must be less than index_type max value" );

        using value_type      = T;
        using size_type       = uint32_t;
        using reference       = T&;
        using const_reference = T const&;
        using iterator        = static_circular_buffer_iterator< T >;
        using const_iterator  = static_circular_buffer_iterator< T const >;

        static_circular_buffer() noexcept = default;

        static_circular_buffer( static_circular_buffer const& other )
        {
                copy_from( other );
        }

        static_circular_buffer( static_circular_buffer&& other ) noexcept
        {
                move_from( other );
                other.clear();
        }

        static_circular_buffer& operator=( static_circular_buffer const& other )
        {
                if ( this == &other )
                        return *this;
                clear();
                copy_from( other );
                return *this;
        }

        static_circular_buffer& operator=( static_circular_buffer&& other ) noexcept
        {
                if ( this == &other )
                        return *this;
                clear();
                move_from( other );
                other.clear();
                return *this;
        }

        /// methods for handling the front side of the circular buffer

        [[nodiscard]] iterator begin()
        {
                return iterator{ storage_.data(), storage_.data() + N, storage_.data() + from_ };
        }

        [[nodiscard]] const_iterator begin() const
        {
                return const_iterator{
                    storage_.data(), storage_.data() + N, storage_.data() + from_ };
        }

        [[nodiscard]] std::reverse_iterator< iterator > rbegin()
        {
                return std::make_reverse_iterator( end() );
        };

        [[nodiscard]] std::reverse_iterator< const_iterator > rbegin() const
        {
                return std::make_reverse_iterator( end() );
        };

        [[nodiscard]] reference front()
        {
                return storage_[static_cast< size_type >( from_ )];
        }

        [[nodiscard]] const_reference front() const
        {
                return storage_[static_cast< size_type >( from_ )];
        }

        [[nodiscard]] T take_front()
        {
                T item = std::move( front() );
                pop_front();
                return item;
        }

        void pop_front()
        {
                storage_.delete_item( static_cast< size_type >( from_ ) );
                from_ = next( from_ );
                if ( from_ == to_ )
                        from_ = -1;
        }

        void pop_front( size_type n )
        {
                for ( size_type i = 0; i < n; i++ )
                        pop_front();
        }

        /// methods for handling the back side of the circular buffer

        [[nodiscard]] iterator end()
        {
                return iterator{ storage_.data(), storage_.data() + N, storage_.data() + to_ };
        }

        [[nodiscard]] const_iterator end() const
        {
                return const_iterator{
                    storage_.data(), storage_.data() + N, storage_.data() + to_ };
        }

        [[nodiscard]] std::reverse_iterator< iterator > rend()
        {
                return std::make_reverse_iterator( begin() );
        };

        [[nodiscard]] std::reverse_iterator< const_iterator > rend() const
        {
                return std::make_reverse_iterator( begin() );
        };

        [[nodiscard]] reference back()
        {
                return storage_[static_cast< size_type >( prev( to_ ) )];
        }

        [[nodiscard]] const_reference back() const
        {
                return storage_[static_cast< size_type >( prev( to_ ) )];
        }

        void push_back( T item )
        {
                emplace_back( std::move( item ) );
        }

        template < typename... Args >
        T& emplace_back( Args&&... args )
        {
                T& ref = storage_.emplace_item(
                    static_cast< size_type >( to_ ), std::forward< Args >( args )... );
                if ( from_ == -1 )
                        from_ = to_;
                to_ = next( to_ );
                return ref;
        }

        /// other methods

        [[nodiscard]] constexpr size_type capacity() const
        {
                return N;
        }

        [[nodiscard]] size_type size() const
        {
                if ( from_ == -1 )
                        return 0;
                auto t = static_cast< size_type >( to_ );
                auto f = static_cast< size_type >( from_ );
                if ( t >= f )
                        return t - f;
                return t + ( N - f );
        }

        [[nodiscard]] bool empty() const
        {
                return from_ == -1;
        }

        [[nodiscard]] bool full() const
        {
                return from_ != -1 && to_ == from_;
        }

        void clear()
        {
                purge();
        }

        ~static_circular_buffer()
        {
                purge();
        }

private:
        static_storage< T, N > storage_;
        index_type             from_ = -1;  /// index of the first item
        index_type             to_   = 0;   /// index past the last item

        void purge()
        {
                while ( !empty() )
                        pop_front();
        }

        void copy_from( static_circular_buffer const& other )
        {
                from_       = 0;
                to_         = static_cast< index_type >( other.size() );
                size_type i = 0;
                for ( T const& item : other )
                        storage_.emplace_item( i++, item );
        }

        void move_from( static_circular_buffer& other ) noexcept
        {
                static_assert( std::is_nothrow_move_constructible_v< T > );
                from_       = 0;
                to_         = static_cast< index_type >( other.size() );
                size_type i = 0;
                for ( T& item : other )
                        storage_.emplace_item( i++, std::move( item ) );
        }

        /// Use this only when moving the indexes in the circular buffer - bullet-proof.
        [[nodiscard]] static constexpr index_type next( index_type const i ) noexcept
        {
                return static_cast< size_type >( i + 1 ) % N;
        }

        [[nodiscard]] static constexpr index_type prev( index_type const i ) noexcept
        {
                return i == 0 ? N - 1 : i - 1;
        }

        template < typename U >
        friend class static_circular_buffer_iterator;
};

template < typename T, std::size_t N >
[[nodiscard]] auto
operator<=>( static_circular_buffer< T, N > const& lh, static_circular_buffer< T, N > const& rh )
{
        return std::lexicographical_compare_three_way( lh.begin(), lh.end(), rh.begin(), rh.end() );
}

template < typename T, std::size_t N >
[[nodiscard]] bool
operator==( static_circular_buffer< T, N > const& lh, static_circular_buffer< T, N > const& rh )
{
        auto size = lh.size();
        if ( size != rh.size() )
                return false;

        return std::equal( lh.begin(), lh.end(), rh.begin() );
}

template < typename T, std::size_t N >
[[nodiscard]] bool
operator!=( static_circular_buffer< T, N > const& lh, static_circular_buffer< T, N > const& rh )
{
        return !( lh == rh );
}

#ifdef EMLABCPP_USE_OSTREAM
/// Output operator for the view, uses comma to separate the items in the view.
template < typename T, std::size_t N >
std::ostream& operator<<( std::ostream& os, static_circular_buffer< T, N > const& cb )
{
        return os << view{ cb };
}
#endif

}  // namespace emlabcpp

template < typename T >
struct std::iterator_traits< emlabcpp::static_circular_buffer_iterator< T > >
{
        using value_type        = T;
        using difference_type   = std::make_signed_t< std::size_t >;
        using pointer           = value_type*;
        using const_pointer     = value_type const*;
        using reference         = value_type&;
        using iterator_category = std::bidirectional_iterator_tag;
};

namespace emlabcpp
{

template < typename T >
class static_circular_buffer_iterator
  : public generic_iterator< static_circular_buffer_iterator< T > >
{
public:
        static constexpr bool is_const = std::is_const_v< T >;
        using value_type               = T;
        using reference       = std::conditional_t< is_const, value_type const&, value_type& >;
        using const_reference = value_type const&;
        using difference_type =
            typename std::iterator_traits< static_circular_buffer_iterator< T > >::difference_type;

        static_circular_buffer_iterator( T* beg, T* end, T* p ) noexcept
          : beg_( beg )
          , end_( end )
          , p_( p )
        {
        }

        static_circular_buffer_iterator( static_circular_buffer_iterator const& ) noexcept =
            default;
        static_circular_buffer_iterator( static_circular_buffer_iterator&& ) noexcept = default;

        static_circular_buffer_iterator&
        operator=( static_circular_buffer_iterator const& ) noexcept = default;
        static_circular_buffer_iterator&
        operator=( static_circular_buffer_iterator&& ) noexcept = default;

        reference operator*() noexcept
        {
                return *p_;
        }

        reference operator*() const noexcept
        {
                return *p_;
        }

        static_circular_buffer_iterator& operator++() noexcept
        {
                p_++;
                if ( p_ == end_ )
                        p_ = beg_;
                return *this;
        }

        static_circular_buffer_iterator& operator--() noexcept
        {
                if ( p_ == beg_ )
                        p_ = end_;
                return *this;
        }

        auto operator<=>( static_circular_buffer_iterator const& other ) const noexcept
        {
                return p_ <=> other.p_;
        }

        bool operator==( static_circular_buffer_iterator const& other ) const noexcept
        {
                return p_ == other.p_;
        }

private:
        T* beg_;
        T* end_;
        T* p_;
};

}  // namespace emlabcpp
