#include "emlabcpp/iterator.h"

#include <new>
#include <type_traits>
#include <utility>

#pragma once

namespace emlabcpp
{

template < typename StorageType, typename T, std::size_t N >
class static_vector_iterator;

/// Data container for up to N elements
template < typename T, std::size_t N >
class static_vector
{

        /// type for storage of one item
        using storage_type = std::aligned_storage_t< sizeof( T ), alignof( T ) >;

        friend class static_vector_iterator< storage_type, T, N >;
        friend class static_vector_iterator< const storage_type, T, N >;

public:
        // public types
        // --------------------------------------------------------------------------------
        using value_type      = T;
        using size_type       = std::size_t;
        using reference       = T&;
        using const_reference = const T&;
        using iterator        = static_vector_iterator< storage_type, T, N >;
        using const_iterator  = static_vector_iterator< const storage_type, T, N >;

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
        }
        template < std::size_t M >
        requires( M <= N ) static_vector( std::array< T, M > data )
        {
                move_from( data );
        }
        static_vector& operator=( const static_vector& other )
        {
                if ( this != &other ) {
                        clear();
                        copy_from( other );
                }
                return *this;
        }
        static_vector& operator=( static_vector&& other ) noexcept
        {
                if ( this != &other ) {
                        clear();
                        move_from( other );
                }
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

        storage_type data_[N];   /// storage of the entire dataset
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
                    reinterpret_cast< T* >( &data_[i] ), std::forward< Args >( args )... );
        }

        template < typename Container >
        void copy_from( const Container& cont )
        {
                for ( const T& item : cont ) {
                        emplace_back( item );
                }
        }

        template < typename Container >
        void move_from( Container&& cont )
        {
                for ( T& item : cont ) {
                        emplace_back( std::move( item ) );
                }
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

template < typename T, std::size_t N >
inline void swap( const static_vector< T, N >& lh, const static_vector< T, N >& rh )
{
        lh.swap( rh );
}

}  // namespace emlabcpp

template < typename StorageType, typename T, std::size_t N >
struct std::iterator_traits< emlabcpp::static_vector_iterator< StorageType, T, N > >
{
        static constexpr bool is_const = std::is_const_v< StorageType >;

        using value_type        = T;
        using difference_type   = std::ptrdiff_t;
        using pointer           = std::conditional_t< is_const, const T*, T* >;
        using const_pointer     = const pointer;
        using reference         = std::conditional_t< is_const, const T&, T& >;
        using iterator_category = std::random_access_iterator_tag;
};

namespace emlabcpp
{

template < typename StorageType, typename T, std::size_t N >
class static_vector_iterator
  : public generic_iterator< static_vector_iterator< StorageType, T, N > >
{
        static constexpr bool is_const = std::is_const_v< StorageType >;
        using storage_type             = StorageType;
        using reference = typename std::iterator_traits< static_vector_iterator >::reference;
        using pointer   = typename std::iterator_traits< static_vector_iterator >::pointer;
        static_vector_iterator( storage_type* ptr )
          : raw_ptr_( ptr )
        {
        }

        friend class static_vector< T, N >;

public:
        static_vector_iterator( const static_vector_iterator& )     = default;
        static_vector_iterator( static_vector_iterator&& ) noexcept = default;

        static_vector_iterator& operator=( const static_vector_iterator& ) = default;
        static_vector_iterator& operator=( static_vector_iterator&& ) = default;

        reference operator*()
        {
                return *reinterpret_cast< pointer >( raw_ptr_ );
        }
        const reference operator*() const
        {
                return *reinterpret_cast< const pointer >( raw_ptr_ );
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

}  // namespace emlabcpp
