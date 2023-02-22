#include <utility>

#pragma once

namespace emlabcpp
{

template < typename T >
class linked_list_node
{
public:
        linked_list_node() = default;

        template < typename... Args >
        explicit linked_list_node( Args&&... args )
          : item_( std::forward< Args >( args )... )
        {
        }

        linked_list_node( const linked_list_node& )            = delete;
        linked_list_node& operator=( const linked_list_node& ) = delete;

        linked_list_node( linked_list_node&& other ) noexcept
        {
                *this = std::move( other );
        }
        linked_list_node& operator=( linked_list_node&& other ) noexcept
        {
                item_ = std::move( other.item_ );

                prev_ = other.prev_;
                if ( prev_ != nullptr ) {
                        prev_->next_ = this;
                        other.prev_  = nullptr;
                }

                next_ = other.next_;
                if ( next_ != nullptr ) {
                        next_->prev_ = this;
                        other.next_  = nullptr;
                }

                return *this;
        }

        void link_as_next( linked_list_node< T >& node )
        {
                node.next_ = next_;
                if ( next_ != nullptr ) {
                        next_->prev_ = &node;
                }

                node.prev_ = this;
                next_      = &node;
        }

        T& operator*()
        {
                return item_;
        }
        const T& operator*() const
        {
                return item_;
        }
        T* operator->()
        {
                return item_;
        }
        const T* operator->() const
        {
                return item_;
        }

        [[nodiscard]] linked_list_node* get_next()
        {
                return next_;
        }

        [[nodiscard]] linked_list_node* get_next( std::size_t id )
        {
                auto* n = this;
                while ( n != nullptr ) {
                        if ( id == 0 ) {
                                return n;
                        }
                        n = n->next_;
                        id -= 1;
                }
                return nullptr;
        }

        [[nodiscard]] std::size_t count_next() const
        {
                std::size_t i = 0;
                auto*       n = next_;
                while ( n != nullptr ) {
                        n = n->next_;
                        i += 1;
                }
                return i;
        }

        [[nodiscard]] linked_list_node* get_prev()
        {
                return prev_;
        }

        [[nodiscard]] linked_list_node* get_prev( const std::size_t id )
        {
                if ( id == 0 ) {
                        return this;
                }
                if ( prev_ == nullptr ) {
                        return nullptr;
                }
                return prev_->get_prev( id - 1 );
        }

        ~linked_list_node()
        {
                if ( prev_ != nullptr ) {
                        prev_->next_ = next_;
                }
                if ( next_ != nullptr ) {
                        next_->prev_ = prev_;
                }
        }

private:
        T                 item_;
        linked_list_node* next_ = nullptr;
        linked_list_node* prev_ = nullptr;
};

}  // namespace emlabcpp
