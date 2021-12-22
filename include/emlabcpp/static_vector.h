#include "emlabcpp/iterator.h"

#include <new>
#include <type_traits>
#include <utility>

#pragma once

namespace emlabcpp
{

template < typename T, std::size_t N >
class static_vector_iterator;

template < typename T, std::size_t N >
class const_static_vector_iterator;

/// Data container for up to N elements, mirroring std::vector behavior.
template < typename T, std::size_t N >
class static_vector
{

        /// type for storage of one item
        using storage_type = std::aligned_storage_t< sizeof( T ), alignof( T ) >;

        friend class static_vector_iterator< T, N >;
        friend class const_static_vector_iterator< T, N >;

public:
        // public types
        // --------------------------------------------------------------------------------
        using value_type      = T;
        using size_type       = std::size_t;
        using reference       = T&;
        using const_reference = const T&;
        using iterator        = static_vector_iterator< T, N >;
        using const_iterator  = const_static_vector_iterator< T, N >;

        // public methods
        // --------------------------------------------------------------------------------
        static_vector() = default;
        static_vector( const static_vector& other )
        {
                for ( size_type i = 0; i < other.size(); ++i ) {
                        push_back( other[i] );
                }
        }
        static_vector( static_vector&& other ) noexcept
        {
                for ( size_type i = 0; i < other.size(); ++i ) {
                        push_back( std::move( other[i] ) );
                }
        }
        static_vector& operator=( const static_vector& other )
        {
                if ( this != &other ) {
                        this->~static_vector();
                        ::new ( this ) static_vector( other );
                }
                return *this;
        }
        static_vector& operator=( static_vector&& other ) noexcept
        {
                if ( this != &other ) {
                        this->~static_vector();
                        ::new ( this ) static_vector( std::move( other ) );
                }
                return *this;
        }

        iterator begin()
        {
                return iterator{ &data_[0] };
        }
        iterator end()
        {
                return iterator{ &data_[size_] };
        }

        const_iterator begin() const
        {
                return const_iterator{ &data_[0] };
        }
        const_iterator end() const
        {
                return const_iterator{ &data_[size_] };
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

        T pop_back()
        {
                T item = std::move( back() );
                delete_item( size_ - 1 );
                size_ -= 1;
                return item;
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

        storage_type data_[N] = { { 0 } };  /// storage of the entire dataset
        size_type    size_    = 0;          /// count of items

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
                    reinterpret_cast< T* >( &data_[i] ), std::forward< Args >( args )... );
        }

        // Reference to the item in data_storage.
        [[nodiscard]] reference ref_item( size_type i )
        {
                return *reinterpret_cast< T* >( &data_[i] );
        }
        [[nodiscard]] const_reference ref_item( size_type i ) const
        {
                return *reinterpret_cast< const T* >( &data_[i] );
        }

        // Cleans entire buffer from items.
        void purge()
        {
                while ( !empty() ) {
                        pop_back();
                }
        }
};

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

}  // namespace emlabcpp

template < typename T, std::size_t N >
struct std::iterator_traits< emlabcpp::static_vector_iterator< T, N > >
{
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = T*;
        using const_pointer     = const T*;
        using reference         = T&;
        using iterator_category = std::random_access_iterator_tag;
};

template < typename T, std::size_t N >
struct std::iterator_traits< emlabcpp::const_static_vector_iterator< T, N > >
{
        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = const T*;
        using const_pointer     = const T*;
        using reference         = const T&;
        using iterator_category = std::random_access_iterator_tag;
};

namespace emlabcpp
{

// TODO: merge the iterators into one impl :/

/// Iterator for static vecotr - nonconst
template < typename T, std::size_t N >
class static_vector_iterator : public generic_iterator< static_vector_iterator< T, N > >
{
        using storage_type = typename static_vector< T, N >::storage_type;

        static_vector_iterator( storage_type* ptr )
          : raw_ptr_( ptr )
        {
        }

        friend class static_vector< T, N >;

public:
        T& operator*()
        {
                return *reinterpret_cast< T* >( raw_ptr_ );
        }
        const T& operator*() const
        {
                return *reinterpret_cast< const T* >( raw_ptr_ );
        }

        static_vector_iterator& operator+=( std::ptrdiff_t offset )
        {
                raw_ptr_ += offset;
                return *this;
        }
        static_vector_iterator& operator-=( std::ptrdiff_t offset )
        {
                raw_ptr_ -= offset;
                return *this;
        }

        auto operator<=>( const static_vector_iterator& other ) const
        {
                return raw_ptr_ <=> other.raw_ptr_;
        }
        bool operator==( const static_vector_iterator& other ) const
        {
                return raw_ptr_ == other.raw_ptr_;
        }

        constexpr std::ptrdiff_t operator-( const static_vector_iterator& other ) const
        {
                return raw_ptr_ - other.raw_ptr_;
        }

private:
        storage_type* raw_ptr_;
};

/// Iterator for static vecotr - const
template < typename T, std::size_t N >
class const_static_vector_iterator : public generic_iterator< const_static_vector_iterator< T, N > >
{
        using storage_type = typename static_vector< T, N >::storage_type;

        const_static_vector_iterator( const storage_type* ptr )
          : raw_ptr_( ptr )
        {
        }

        friend class static_vector< T, N >;

public:
        const T& operator*()
        {
                return *reinterpret_cast< const T* >( raw_ptr_ );
        }
        const T& operator*() const
        {
                return *reinterpret_cast< const T* >( raw_ptr_ );
        }

        const_static_vector_iterator& operator+=( std::ptrdiff_t offset )
        {
                raw_ptr_ += offset;
                return *this;
        }
        const_static_vector_iterator& operator-=( std::ptrdiff_t offset )
        {
                raw_ptr_ -= offset;
                return *this;
        }

        auto operator<=>( const const_static_vector_iterator& other ) const
        {
                return raw_ptr_ <=> other.raw_ptr_;
        }
        bool operator==( const const_static_vector_iterator& other ) const
        {
                return raw_ptr_ == other.raw_ptr_;
        }

        constexpr std::ptrdiff_t operator-( const const_static_vector_iterator& other ) const
        {
                return raw_ptr_ - other.raw_ptr_;
        }

private:
        const storage_type* raw_ptr_;
};

}  // namespace emlabcpp
