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

#include <atomic>
#include <limits>
#include <ranges>

namespace emlabcpp
{

template < typename T >
class static_circular_buffer_iterator;

template < std::size_t N >
constexpr auto _select_index_type()
{
        if constexpr ( N <= std::numeric_limits< std::uint8_t >::max() )
                return std::atomic_uint8_t{};
        else if constexpr ( N <= std::numeric_limits< std::uint16_t >::max() )
                return std::atomic_uint16_t{};
        else if constexpr ( N <= std::numeric_limits< std::uint32_t >::max() )
                return std::atomic_uint32_t{};
        else
                return std::atomic_uint64_t{};
}

/// Strategy for managing indices in circular buffer via empty slot
///
/// This strategy uses one slot as empty slot to distinguish between full and empty buffer.
/// Thus the maximum number of storable elements is N-1 where N is the size of the buffer.
///
template < std::size_t N >
struct _spacer_strategy
{
        static constexpr std::size_t max_size = N + 1;

        using index_type = decltype( _select_index_type< N >() );
        using size_type  = typename index_type::value_type;

        void incr_front() noexcept
        {
                from_ = static_cast< size_type >( ( from_ + 1 ) % max_size );
        }

        void incr_front( size_type n ) noexcept
        {
                from_ = static_cast< size_type >( ( from_ + n ) % max_size );
        }

        auto front_idx() const noexcept
        {
                return from_.load();
        }

        void incr_back() noexcept
        {
                to_ = static_cast< size_type >( ( to_ + 1 ) % max_size );
        }

        void incr_back( size_type n ) noexcept
        {
                to_ = ( to_ + n ) % max_size;
        }

        auto back_idx() const noexcept
        {
                return to_.load();
        }

        size_type size() const noexcept
        {
                if ( to_ >= from_ )
                        return to_ - from_;
                return static_cast< size_type >( to_ + ( max_size - from_ ) );
        }

        bool empty() const noexcept
        {
                return from_ == to_;
        }

        bool full() const noexcept
        {
                return size() == N;
        }

        index_type last_idx() const noexcept
        {
                return ( to_ + max_size - 1 ) % max_size;
        }

private:
        index_type from_ = 0;
        index_type to_   = 0;
};

/// Strategy for managing indices in circular buffer via overflowing indices
///
/// This strategy allows to use all N slots of the buffer by allowing the indices to overflow.
/// This works correctly as long as buffer size is power of two.
///
template < std::size_t N >
struct _overflow_strategy
{
        static_assert( N && !( N & ( N - 1 ) ), "N must be power of two" );
        static constexpr std::size_t max_size = N;

        using index_type = decltype( _select_index_type< N >() );
        using size_type  = typename index_type::value_type;

        void incr_front() noexcept
        {
                from_ = from_ + 1;
        }

        void incr_front( size_type n ) noexcept
        {
                from_ = from_ + n;
        }

        auto front_idx() const noexcept
        {
                return from_.load() % max_size;
        }

        void incr_back() noexcept
        {
                to_ = to_ + 1;
        }

        void incr_back( size_type n ) noexcept
        {
                to_ = to_ + n;
        }

        auto back_idx() const noexcept
        {
                return to_.load() % max_size;
        }

        auto size() const noexcept
        {
                return static_cast< size_type >( to_ - from_ );
        }

        bool empty() const noexcept
        {
                return from_ == to_;
        }

        bool full() const noexcept
        {
                return size() == static_cast< size_type >( max_size );
        }

        index_type last_idx() const noexcept
        {
                return ( to_ + max_size - 1 ) % max_size;
        }

private:
        index_type from_ = 0;
        index_type to_   = 0;
};

template < typename T, std::size_t N >
auto _select_strategy()
{
        static constexpr bool is_power_of_two = N && !( N & ( N - 1 ) );
        if constexpr ( is_power_of_two )
                return _overflow_strategy< N >{};
        else
                return _spacer_strategy< N >{};
}

/// Class implementing circular buffer of any type for up to N elements.
///
/// It is safe in "single consumer single producer" scenario between main loop and interrupts.
/// Because of that the behavior is as follows:
///  - on insertion, item is inserted and than index is advanced
///  - on removal, item is removed and than index is advanced
///
template < typename T, std::size_t N, typename Strategy = decltype( _select_strategy< T, N >() ) >
struct static_circular_buffer
{
        static constexpr std::size_t max_size = Strategy::max_size;

        using index_type = typename Strategy::index_type;
        using size_type  = typename Strategy::size_type;

        static_assert(
            N < std::numeric_limits< typename index_type::value_type >::max(),
            "static_circular_buffer: N must be less than index_type max value" );

        using value_type      = T;
        using reference       = T&;
        using const_reference = T const&;
        using iterator        = static_circular_buffer_iterator< T >;
        using const_iterator  = static_circular_buffer_iterator< T const >;

        /// Default constructed circular buffer is empty
        static_circular_buffer() noexcept = default;

        static_circular_buffer( static_circular_buffer const& other )
        {
                copy_range_back( other );
        }

        static_circular_buffer( static_circular_buffer&& other ) noexcept
        {
                move_range_back( other );
                other.clear();
        }

        /// Clears the buffer by destructing all contained elements and copies
        /// from other buffer.
        static_circular_buffer& operator=( static_circular_buffer const& other )
        {
                if ( this == &other )
                        return *this;
                clear();
                copy_range_back( other );
                return *this;
        }

        /// Clears the buffer by destructing all contained elements and moves
        /// from other buffer.
        static_circular_buffer& operator=( static_circular_buffer&& other ) noexcept
        {
                if ( this == &other )
                        return *this;
                clear();
                move_range_back( other );
                other.clear();
                return *this;
        }

        /// Iterator to first element
        [[nodiscard]] iterator begin() noexcept
        {
                return iterator{
                    storage_.data(),
                    storage_.data() + max_size,
                    storage_.data() + strategy_.front_idx() };
        }

        /// Iterator to first element
        [[nodiscard]] const_iterator begin() const noexcept
        {
                return const_iterator{
                    storage_.data(),
                    storage_.data() + max_size,
                    storage_.data() + strategy_.front_idx() };
        }

        /// Reverse iterator to last element
        [[nodiscard]] std::reverse_iterator< iterator > rbegin() noexcept
        {
                return std::make_reverse_iterator( end() );
        };

        /// Reverse iterator to last element
        [[nodiscard]] std::reverse_iterator< const_iterator > rbegin() const noexcept
        {
                return std::make_reverse_iterator( end() );
        };

        /// Reference to first element
        [[nodiscard]] reference front() noexcept
        {
                return storage_[static_cast< size_type >( strategy_.front_idx() )];
        }

        /// Reference to first element
        [[nodiscard]] const_reference front() const noexcept
        {
                return storage_[static_cast< size_type >( strategy_.front_idx() )];
        }

        /// Removes and returns first element, is safe in SPSC scenario
        [[nodiscard]] T take_front()
        {
                T item = std::move( front() );
                pop_front();
                return item;
        }

        /// Removes and moves first n elements into provided buffer, is safe in SPSC scenario
        void take_front( auto&& buffer )
        {
                auto  n        = static_cast< size_type >( std::size( buffer ) );
                auto  iter     = std::begin( buffer );
                auto  idx      = static_cast< size_type >( strategy_.front_idx() );
                auto* p        = storage_.data() + idx;
                auto  capacity = static_cast< size_type >( max_size - idx );
                if ( capacity >= n ) {
                        std::move( p, p + n, iter );
                } else {
                        iter = std::move( p, p + capacity, iter );
                        std::move( storage_.data(), storage_.data() + idx + n - capacity, iter );
                }
                strategy_.incr_front( n );
        }

        /// Removes first element, is safe in SPSC scenario
        void pop_front()
        {
                storage_.delete_item( static_cast< size_type >( strategy_.front_idx() ) );
                strategy_.incr_front();
        }

        /// Removes first n elements, is safe in SPSC scenario
        void pop_front( size_type n )
        {
                auto idx      = static_cast< size_type >( strategy_.front_idx() );
                auto capacity = max_size - idx;
                if ( capacity >= n ) {
                        storage_.delete_n( idx, n );
                } else {
                        storage_.delete_n( idx, capacity );
                        storage_.delete_n( 0, n - capacity );
                }
                strategy_.incr_front( n );
        }

        /// Iterator to one-past-last element
        [[nodiscard]] iterator end() noexcept
        {
                return iterator{
                    storage_.data(),
                    storage_.data() + max_size,
                    storage_.data() + strategy_.back_idx() };
        }

        /// Iterator to one-past-last element
        [[nodiscard]] const_iterator end() const noexcept
        {
                return const_iterator{
                    storage_.data(),
                    storage_.data() + max_size,
                    storage_.data() + strategy_.back_idx() };
        }

        /// Reverse iterator to one-before-first element
        [[nodiscard]] std::reverse_iterator< iterator > rend() noexcept
        {
                return std::make_reverse_iterator( begin() );
        };

        /// Reverse iterator to one-before-first element
        [[nodiscard]] std::reverse_iterator< const_iterator > rend() const noexcept
        {
                return std::make_reverse_iterator( begin() );
        };

        /// Reference to last element
        [[nodiscard]] reference back() noexcept
        {
                return storage_[static_cast< size_type >( strategy_.last_idx() )];
        }

        /// Reference to last element
        [[nodiscard]] const_reference back() const noexcept
        {
                return storage_[static_cast< size_type >( strategy_.last_idx() )];
        }

        /// Inserts item at the back, is safe in SPSC scenario
        void push_back( T item )
        {
                emplace_back( std::move( item ) );
        }

        /// Inserts item constructed with args... at the back, is safe in SPSC scenario
        template < typename... Args >
        T& emplace_back( Args&&... args )
        {
                T& ref = storage_.emplace_item(
                    static_cast< size_type >( strategy_.back_idx() ),
                    std::forward< Args >( args )... );
                strategy_.incr_back();
                return ref;
        }

        /// Inserts range by copy at the back, is safe in SPSC scenario
        void copy_range_back( auto&& range )
        {
                auto idx      = static_cast< size_type >( strategy_.back_idx() );
                auto capacity = static_cast< size_type >( max_size - idx );
                auto n        = std::size( range );
                auto iter     = std::begin( range );
                if ( capacity >= n ) {
                        storage_.copy_n( idx, n, iter );
                } else {
                        iter = storage_.copy_n( idx, capacity, iter );
                        storage_.copy_n( 0, n - capacity, iter );
                }
                strategy_.incr_back( static_cast< size_type >( n ) );
        }

        /// Inserts range by move at the back, is safe in SPSC scenario
        void move_range_back( auto&& range )
        {
                auto idx      = static_cast< size_type >( strategy_.back_idx() );
                auto capacity = static_cast< size_type >( max_size - idx );
                auto n        = std::size( range );
                auto iter     = std::begin( range );
                if ( capacity >= n ) {
                        storage_.move_n( idx, n, iter );
                } else {
                        iter = storage_.move_n( idx, capacity, iter );
                        storage_.move_n( 0, n - capacity, iter );
                }
                strategy_.incr_back( static_cast< size_type >( n ) );
        }

        /// Returns maximum capacity of the buffer
        [[nodiscard]] constexpr size_type capacity() const noexcept
        {
                return N;
        }

        /// Returns current size of the buffer
        [[nodiscard]] size_type size() const
        {
                return static_cast< size_type >( strategy_.size() );
        }

        /// Returns whether the buffer is empty
        [[nodiscard]] bool empty() const noexcept
        {
                return strategy_.empty();
        }

        /// Returns whether the buffer is full
        [[nodiscard]] bool full() const noexcept
        {
                return strategy_.full();
        }

        /// Clears the buffer by destructing all contained elements
        void clear()
        {
                pop_front( size() );
        }

        /// Destructor destructs all contained elements
        ~static_circular_buffer()
        {
                clear();
        }

private:
        static_storage< T, max_size > storage_;
        Strategy                      strategy_;

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

        static_circular_buffer_iterator& operator+=( difference_type v ) noexcept
        {
                p_ += v;
                while ( p_ >= end_ )
                        p_ -= ( end_ - beg_ );
                return *this;
        }

        static_circular_buffer_iterator& operator--() noexcept
        {
                p_--;
                if ( p_ == ( beg_ - 1 ) )
                        p_ = end_ - 1;
                return *this;
        }

        static_circular_buffer_iterator& operator-=( difference_type v ) noexcept
        {
                p_ -= v;
                while ( p_ < beg_ )
                        p_ += ( end_ - beg_ );
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
