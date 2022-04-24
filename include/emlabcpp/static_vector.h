#include "emlabcpp/concepts.h"
#include "emlabcpp/iterator.h"

#include <new>
#include <type_traits>
#include <utility>

#pragma once

namespace emlabcpp
{

/// Data container for up to N elements
template < typename T, std::size_t N >
class static_vector
{

        /// type for storage of one item
        using storage_type = std::aligned_storage_t< sizeof( T ) * N, alignof( T ) >;

public:
        static constexpr std::size_t capacity = N;

        // public types
        // --------------------------------------------------------------------------------
        using value_type      = T;
        using size_type       = std::size_t;
        using reference       = T&;
        using const_reference = const T&;
        using iterator        = T*;
        using const_iterator  = const T*;

        // public methods
        // --------------------------------------------------------------------------------
        static_vector() = default;
        static_vector( const static_vector& other )
        {
                copy_from( other );
        }
        static_vector( static_vector&& other ) noexcept
        {
                move_from( other );
                other.clear();
        }
        template < std::size_t M >
        requires( M <= N ) static_vector( std::array< T, M > data )
        {
                move_from( data );
        }
        static_vector& operator=( const static_vector& other )
        {
                if ( this == &other ) {
                        return *this;
                }
                clear();
                copy_from( other );
                return *this;
        }
        static_vector& operator=( static_vector&& other ) noexcept
        {
                if ( this == &other ) {
                        return *this;
                }
                clear();
                move_from( other );
                other.clear();
                return *this;
        }

        void swap( static_vector& other )
        {
                using std::swap;
                size_type shared_n = std::min( size(), other.size() );
                for ( size_type i = 0; i < shared_n; ++i ) {
                        swap( ref_item( i ), other.ref_item( i ) );
                }
                for ( size_type i = shared_n; i < size(); ++i ) {
                        other.emplace_item( i, std::move( ref_item( i ) ) );
                }
                for ( size_type i = shared_n; i < other.size(); ++i ) {
                        emplace_item( i, std::move( other.ref_item( i ) ) );
                }
        }

        [[nodiscard]] iterator begin()
        {
                return reinterpret_cast< T* >( &data_ );
        }

        [[nodiscard]] iterator end()
        {
                return begin() + size_;
        }

        [[nodiscard]] const_iterator begin() const
        {
                return reinterpret_cast< const T* >( &data_ );
        }

        [[nodiscard]] const_iterator end() const
        {
                return begin() + size_;
        }

        [[nodiscard]] reference front()
        {
                return ref_item( 0 );
        }

        [[nodiscard]] const_reference front() const
        {
                return ref_item( 0 );
        }

        void push_back( T item )
        {
                emplace_back( std::move( item ) );
        }

        template < typename... Args >
        void emplace_back( Args&&... args )
        {
                emplace_item( size_, std::forward< Args >( args )... );
                size_ += 1;
        }

        [[nodiscard]] T take_back()
        {
                T item = std::move( back() );
                pop_back();
                return item;
        }

        void pop_back()
        {
                delete_item( size_ - 1 );
                size_ -= 1;
        }

        [[nodiscard]] reference back()
        {
                return ref_item( size_ - 1 );
        }
        [[nodiscard]] const_reference back() const
        {
                return ref_item( size_ - 1 );
        }

        // other methods

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

        const_reference operator[]( size_type i ) const
        {
                return ref_item( i );
        }
        reference operator[]( size_type i )
        {
                return ref_item( i );
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
        // private attributes
        // --------------------------------------------------------------------------------

        storage_type data_;      /// storage of the entire dataset
        size_type    size_ = 0;  /// count of items

        // private methods
        // --------------------------------------------------------------------------------
        void delete_item( size_type i )
        {
                std::destroy_at( std::addressof( ref_item( i ) ) );
        }

        template < typename... Args >
        void emplace_item( size_type i, Args&&... args )
        {
                std::construct_at(
                    reinterpret_cast< T* >( &data_ ) + i, std::forward< Args >( args )... );
        }

        template < typename Container >
        void copy_from( const Container& cont )
        {
                size_ = cont.size();
                std::uninitialized_copy(
                    cont.begin(), cont.end(), reinterpret_cast< T* >( &data_ ) );
        }

        template < typename Container >
        void move_from( Container& cont )
        {
                size_ = cont.size();
                std::uninitialized_move(
                    cont.begin(), cont.end(), reinterpret_cast< T* >( &data_ ) );
        }

        // Reference to the item in data_storage.
        [[nodiscard]] reference ref_item( size_type i )
        {
                return *( begin() + i );
        }
        [[nodiscard]] const_reference ref_item( size_type i ) const
        {
                return *( begin() + i );
        }

        // Cleans entire buffer from items.
        void purge()
        {
                while ( size_ > 0 ) {
                        delete_item( size_ - 1 );
                        size_ -= 1;
                }
        }
};

template < typename T, std::size_t N >
[[nodiscard]] inline auto
operator<=>( const static_vector< T, N >& lh, const static_vector< T, N >& rh )
{
        return std::lexicographical_compare_three_way( lh.begin(), lh.end(), rh.begin(), rh.end() );
}

template < typename T, std::size_t N >
[[nodiscard]] inline bool
operator==( const static_vector< T, N >& lh, const static_vector< T, N >& rh )
{
        auto size = lh.size();
        if ( size != rh.size() ) {
                return false;
        }

        for ( std::size_t i = 0; i < size; ++i ) {
                if ( lh[i] != rh[i] ) {
                        return false;
                }
        }
        return true;
}

template < typename T, std::size_t N >
[[nodiscard]] inline bool
operator!=( const static_vector< T, N >& lh, const static_vector< T, N >& rh )
{
        return !( lh == rh );
}

template < typename T, std::size_t N >
inline void swap( const static_vector< T, N >& lh, const static_vector< T, N >& rh )
{
        lh.swap( rh );
}

// Output operator for the view, uses comma to separate the items in the view.
template < ostreamlike Stream, typename T, std::size_t N >
inline auto& operator<<( Stream& os, const static_vector< T, N >& vec )
{
        return os << view{ vec };
}

}  // namespace emlabcpp
