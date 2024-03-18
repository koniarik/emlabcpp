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

#include "emlabcpp/iterator.h"
#include "emlabcpp/static_storage.h"

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

public:
        static constexpr std::size_t capacity = N;

        /// public types
        /// --------------------------------------------------------------------------------
        using value_type      = T;
        using size_type       = std::size_t;
        using reference       = T&;
        using const_reference = T const&;
        using iterator        = static_circular_buffer_iterator< static_circular_buffer< T, N > >;
        using const_iterator =
            static_circular_buffer_iterator< static_circular_buffer< T, N > const >;

        /// public methods
        /// --------------------------------------------------------------------------------
        static_circular_buffer() = default;

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
                return storage_[from_];
        }

        [[nodiscard]] const_reference front() const
        {
                return storage_[from_];
        }

        [[nodiscard]] T take_front()
        {
                T item = std::move( front() );
                pop_front();
                return item;
        }

        void pop_front()
        {
                storage_.delete_item( from_ );
                from_ = next( from_ );
        }

        void pop_front( std::size_t n )
        {
                for ( std::size_t i = 0; i < n; i++ )
                        pop_front();
        }

        /// methods for handling the back side of the circular buffer

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
                return storage_[prev( to_ )];
        }

        [[nodiscard]] const_reference back() const
        {
                return storage_[prev( to_ )];
        }

        void push_back( T item )
        {
                emplace_back( std::move( item ) );
        }

        template < typename... Args >
        void emplace_back( Args&&... args )
        {
                storage_.emplace_item( to_, std::forward< Args >( args )... );
                to_ = next( to_ );
        }

        /// other methods

        [[nodiscard]] constexpr std::size_t max_size() const
        {
                return N;
        }

        [[nodiscard]] std::size_t size() const
        {
                if ( to_ >= from_ )
                        return to_ - from_;
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

        const_reference operator[]( size_type const i ) const
        {
                return storage_[( from_ + i ) % real_size];
        }

        reference operator[]( size_type const i )
        {
                return storage_[( from_ + i ) % real_size];
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
        /// private attributes
        /// --------------------------------------------------------------------------------

        static_storage< T, real_size > storage_;
        size_type                      from_ = 0;  /// index of the first item
        size_type                      to_   = 0;  /// index past the last item

        /// from_ == to_ means empty
        /// to_ + 1 == from_ is full

        /// private methods
        /// --------------------------------------------------------------------------------
        //

        /// Cleans entire buffer from items.
        void purge()
        {
                while ( !empty() )
                        pop_front();
        }

        void copy_from( static_circular_buffer const& other )
        {
                to_ = other.size();
                std::uninitialized_copy( other.begin(), other.end(), storage_.data() );
        }

        void move_from( static_circular_buffer& other )
        {
                to_ = other.size();
                std::uninitialized_move( other.begin(), other.end(), storage_.data() );
        }

        /// Use this only when moving the indexes in the circular buffer - bullet-proof.
        [[nodiscard]] constexpr auto next( size_type const i ) const
        {
                return ( i + 1 ) % real_size;
        }

        [[nodiscard]] constexpr auto prev( size_type const i ) const
        {
                return i == 0 ? real_size - 1 : i - 1;
        }

        template < typename Container >
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

        for ( std::size_t i = 0; i < size; ++i )
                if ( lh[i] != rh[i] )
                        return false;
        return true;
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

template < typename Container >
struct std::iterator_traits< emlabcpp::static_circular_buffer_iterator< Container > >
{
        using value_type        = typename Container::value_type;
        using difference_type   = std::make_signed_t< std::size_t >;
        using pointer           = value_type*;
        using const_pointer     = value_type const*;
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
        using reference       = std::conditional_t< is_const, value_type const&, value_type& >;
        using const_reference = value_type const&;
        using difference_type = typename std::iterator_traits<
            static_circular_buffer_iterator< Container > >::difference_type;

        static_circular_buffer_iterator( Container& cont, std::size_t i ) noexcept
          : cont_( cont )
          , i_( i )
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
                return cont_.get().storage_[i_];
        }

        reference operator*() const noexcept
        {
                return cont_.get().storage_[i_];
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

        auto operator<=>( static_circular_buffer_iterator const& other ) const noexcept
        {
                return i_ <=> other.i_;
        }

        bool operator==( static_circular_buffer_iterator const& other ) const noexcept
        {
                return i_ == other.i_;
        }

        difference_type operator-( static_circular_buffer_iterator const& other ) const noexcept
        {
                std::size_t i = i_;
                if ( i < cont_.get().from_ )
                        i += real_size;
                std::size_t j = other.i_;
                if ( j < cont_.get().from_ )
                        j += real_size;
                return static_cast< difference_type >( i - j );
        }

private:
        std::reference_wrapper< Container > cont_;
        std::size_t                         i_;
};

}  // namespace emlabcpp
