#include "emlabcpp/iterator.h"

#include <cstdint>
#include <limits>
#include <new>
#include <type_traits>
#include <utility>

#ifdef EMLABCPP_USE_STREAMS
#include <ostream>
#endif

#pragma once

namespace emlabcpp
{

template < typename Container >
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
        /// We need real_size of the buffer to be +1 bigger than number of items
        static constexpr std::size_t real_size = N + 1;
        /// type for storage of one item
        using storage_type = std::aligned_storage_t< sizeof( T ) * real_size, alignof( T ) >;

public:
        static constexpr std::size_t capacity = N;

        // public types
        // --------------------------------------------------------------------------------
        using value_type      = T;
        using size_type       = std::size_t;
        using reference       = T&;
        using const_reference = const T&;
        using iterator        = static_circular_buffer_iterator< static_circular_buffer< T, N > >;
        using const_iterator =
            static_circular_buffer_iterator< const static_circular_buffer< T, N > >;

        // public methods
        // --------------------------------------------------------------------------------
        static_circular_buffer() = default;
        static_circular_buffer( const static_circular_buffer& other )
        {
                copy_from( other );
        }
        static_circular_buffer( static_circular_buffer&& other ) noexcept(
            std::is_nothrow_move_constructible_v< T > )
        {
                move_from( other );
                other.clear();
        }
        static_circular_buffer& operator=( const static_circular_buffer& other )
        {
                if ( this == &other ) {
                        return *this;
                }
                clear();
                copy_from( other );
                return *this;
        }
        static_circular_buffer& operator=( static_circular_buffer&& other ) noexcept(
            std::is_nothrow_move_assignable_v< T > )
        {
                if ( this == &other ) {
                        return *this;
                }
                clear();
                move_from( other );
                other.clear();
                return *this;
        }
        // methods for handling the front side of the circular buffer

        [[nodiscard]] iterator begin()
        {
                return iterator{ *this, from_ };
        }
        [[nodiscard]] const_iterator begin() const
        {
                return const_iterator{ *this, from_ };
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
                return ref_item( from_ );
        }
        [[nodiscard]] const_reference front() const
        {
                return ref_item( from_ );
        }

        [[nodiscard]] T take_front()
        {
                T item = std::move( front() );
                pop_front();
                return item;
        }

        void pop_front()
        {
                delete_item( from_ );
                from_ = next( from_ );
        }

        // methods for handling the back side of the circular buffer

        [[nodiscard]] iterator end()
        {
                return iterator{ *this, to_ };
        }
        [[nodiscard]] const_iterator end() const
        {
                return const_iterator{ *this, to_ };
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
                return ref_item( prev( to_ ) );
        }
        [[nodiscard]] const_reference back() const
        {
                return ref_item( prev( to_ ) );
        }

        void push_back( T item )
        {
                emplace_back( std::move( item ) );
        }

        template < typename... Args >
        void emplace_back( Args&&... args )
        {
                emplace_item( to_, std::forward< Args >( args )... );
                to_ = next( to_ );
        }

        // other methods

        [[nodiscard]] constexpr std::size_t max_size() const
        {
                return N;
        }

        [[nodiscard]] std::size_t size() const
        {
                if ( to_ >= from_ ) {
                        return to_ - from_;
                }
                return to_ + ( real_size - from_ );
        }

        [[nodiscard]] bool empty() const
        {
                return to_ == from_;
        }

        [[nodiscard]] bool full() const
        {
                return next( to_ ) == from_;
        }

        const_reference operator[]( size_type i ) const
        {
                return ref_item( ( from_ + i ) % real_size );
        }
        reference operator[]( size_type i )
        {
                return ref_item( ( from_ + i ) % real_size );
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
        // private attributes
        // --------------------------------------------------------------------------------

        storage_type data_ = { { 0 } };  // storage of the entire dataset
        size_type    from_ = 0;          // index of the first item
        size_type    to_   = 0;          // index past the last item

        // from_ == to_ means empty
        // to_ + 1 == from_ is full

        // private methods
        // --------------------------------------------------------------------------------
        //
        // Set of [delete,init,emplace]_item methods is necessary as data_ is not array of T, but
        // array of byte-like-type that can store T -> T does not have to be initialized there. We
        // want to fully support T objects - their constructors/destructors are correctly called and
        // we do not require default constructor. This implies that data_ has some slots
        // un-initialized, some are initialized and we have to handle them correctly.
        //
        // All three methods are used to handle this part of the objects in this scenario, that
        // requires features of C++ we do not want to replicate and it's bettter to hide them in
        // methods.

        void delete_item( size_type i )
        {
                std::destroy_at( std::addressof( ref_item( i ) ) );
        }

        template < typename... Args >
        void emplace_item( size_type i, Args&&... args )
        {
                std::construct_at(
                    reinterpret_cast< T* >( std::addressof( data_ ) ) + i,
                    std::forward< Args >( args )... );
        }

        // Reference to the item in data_storage.
        [[nodiscard]] reference ref_item( size_type i )
        {
                return *( reinterpret_cast< T* >( std::addressof( data_ ) ) + i );
        }
        [[nodiscard]] const_reference ref_item( size_type i ) const
        {
                return *( reinterpret_cast< const T* >( std::addressof( data_ ) ) + i );
        }

        // Cleans entire buffer from items.
        void purge()
        {
                while ( !empty() ) {
                        pop_front();
                }
        }

        void copy_from( const static_circular_buffer& other )
        {
                to_ = other.size();
                std::uninitialized_copy(
                    other.begin(), other.end(), reinterpret_cast< T* >( std::addressof( data_ ) ) );
        }

        void move_from( static_circular_buffer& other )
        {
                to_ = other.size();
                std::uninitialized_move(
                    other.begin(), other.end(), reinterpret_cast< T* >( std::addressof( data_ ) ) );
        }

        // Use this only when moving the indexes in the circular buffer - bullet-proof.
        [[nodiscard]] constexpr auto next( size_type i ) const
        {
                return ( i + 1 ) % real_size;
        }
        [[nodiscard]] constexpr auto prev( size_type i ) const
        {
                return i == 0 ? real_size - 1 : i - 1;
        }

        template < typename Container >
        friend class static_circular_buffer_iterator;
};

template < typename T, std::size_t N >
[[nodiscard]] inline auto
operator<=>( const static_circular_buffer< T, N >& lh, const static_circular_buffer< T, N >& rh )
{
        return std::lexicographical_compare_three_way( lh.begin(), lh.end(), rh.begin(), rh.end() );
}

template < typename T, std::size_t N >
[[nodiscard]] inline bool
operator==( const static_circular_buffer< T, N >& lh, const static_circular_buffer< T, N >& rh )
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
operator!=( const static_circular_buffer< T, N >& lh, const static_circular_buffer< T, N >& rh )
{
        return !( lh == rh );
}

#ifdef EMLABCPP_USE_STREAMS
// Output operator for the view, uses comma to separate the items in the view.
template < typename T, std::size_t N >
inline std::ostream& operator<<( std::ostream& os, const static_circular_buffer< T, N >& cb )
{
        return os << view{ cb };
}
#endif

}  // namespace emlabcpp

template < typename Container >
struct std::iterator_traits< emlabcpp::static_circular_buffer_iterator< Container > >
{
        using value_type        = typename Container::value_type;
        using difference_type   = std::make_signed_t< std::size_t >;
        using pointer           = value_type*;
        using const_pointer     = const value_type*;
        using reference         = value_type&;
        using iterator_category = std::random_access_iterator_tag;
};

namespace emlabcpp
{

template < typename Container >
class static_circular_buffer_iterator
  : public generic_iterator< static_circular_buffer_iterator< Container > >
{
public:
        static constexpr bool        is_const  = std::is_const_v< Container >;
        static constexpr std::size_t real_size = Container::real_size;
        using value_type                       = typename Container::value_type;
        using reference       = std::conditional_t< is_const, const value_type&, value_type& >;
        using difference_type = typename std::iterator_traits<
            static_circular_buffer_iterator< Container > >::difference_type;

        static_circular_buffer_iterator( Container& cont, std::size_t i ) noexcept
          : cont_( cont )
          , i_( i )
        {
        }

        static_circular_buffer_iterator( const static_circular_buffer_iterator& ) noexcept =
            default;
        static_circular_buffer_iterator( static_circular_buffer_iterator&& ) noexcept = default;

        static_circular_buffer_iterator&
        operator=( const static_circular_buffer_iterator& ) noexcept = default;
        static_circular_buffer_iterator&
        operator=( static_circular_buffer_iterator&& ) noexcept = default;

        reference operator*() noexcept
        {
                return cont_.ref_item( i_ );
        }

        const reference operator*() const noexcept
        {
                return cont_.ref_item( i_ );
        }

        static_circular_buffer_iterator& operator+=( difference_type j ) noexcept
        {
                auto uj = static_cast< std::size_t >( j );
                i_      = ( i_ + uj ) % real_size;
                return *this;
        }

        static_circular_buffer_iterator& operator-=( difference_type j ) noexcept
        {
                auto uj = static_cast< std::size_t >( j );
                i_      = ( i_ + real_size - uj ) % real_size;
                return *this;
        }

        auto operator<=>( const static_circular_buffer_iterator& other ) const noexcept
        {
                return i_ <=> other.i_;
        }

        bool operator==( const static_circular_buffer_iterator& other ) const noexcept
        {
                return i_ == other.i_;
        }

        difference_type operator-( const static_circular_buffer_iterator& other ) const noexcept
        {
                std::size_t i = i_;
                if ( i < cont_.from_ ) {
                        i += real_size;
                }
                std::size_t j = other.i_;
                if ( j < cont_.from_ ) {
                        j += real_size;
                }
                return static_cast< difference_type >( i - j );
        }

private:
        Container&  cont_;
        std::size_t i_;
};

}  // namespace emlabcpp
