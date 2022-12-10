#include <utility>

#pragma once

namespace emlabcpp
{

template < typename Derived >
class binding_linked_list_node
{
public:
        binding_linked_list_node() = default;

        binding_linked_list_node( const binding_linked_list_node& )            = delete;
        binding_linked_list_node& operator=( const binding_linked_list_node& ) = delete;

        binding_linked_list_node( binding_linked_list_node&& other ) noexcept
        {
                *this = std::move( other );
        }
        binding_linked_list_node& operator=( binding_linked_list_node&& other ) noexcept
        {
                if ( other.prev != nullptr ) {
                        prev       = other.prev;
                        prev->next = this;
                }
                if ( other.next != nullptr ) {
                        next       = other.next;
                        next->prev = this;
                }
                return *this;
        }

        void push_after( binding_linked_list_node< Derived >* node )
        {
                node->next = next;
                if ( next != nullptr ) {
                        next->prev = node;
                }

                node->prev = this;
                next       = node;
        }

        [[nodiscard]] Derived* get_next()
        {
                if ( next == nullptr ) {
                        return nullptr;
                }
                return &next->get_derived();
        }

        [[nodiscard]] Derived* get_next( std::size_t id )
        {
                if ( id == 0 ) {
                        return &get_derived();
                }
                if ( next == nullptr ) {
                        return nullptr;
                }
                return next->get_next( id - 1 );
        }

        [[nodiscard]] std::size_t count_next() const
        {
                std::size_t i = 0;
                auto*       n = next;
                while ( n != nullptr ) {
                        n = n->next;
                        i += 1;
                }
                return i;
        }

        [[nodiscard]] Derived* get_prev()
        {
                if ( prev == nullptr ) {
                        return nullptr;
                }
                return &prev->get_derived();
        }

        [[nodiscard]] Derived* get_prev( std::size_t id )
        {
                if ( id == 0 ) {
                        return &get_derived();
                }
                if ( prev == nullptr ) {
                        return nullptr;
                }
                return get_prev( id - 1 );
        }

        ~binding_linked_list_node()
        {
                if ( prev != nullptr ) {
                        prev->next = next;
                }
                if ( next != nullptr ) {
                        next->prev = prev;
                }
        }

private:
        binding_linked_list_node< Derived >* next = nullptr;
        binding_linked_list_node< Derived >* prev = nullptr;

        [[nodiscard]] Derived& get_derived()
        {
                return *static_cast< Derived* >( this );
        }
        [[nodiscard]] const Derived& get_derived() const
        {
                return *static_cast< const Derived* >( this );
        }
};

}  // namespace emlabcpp
