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

#include <utility>

#pragma once

namespace emlabcpp
{

template < typename Base >
class linked_list_node_base
{
public:
        linked_list_node_base() = default;

        linked_list_node_base( const linked_list_node_base& )            = delete;
        linked_list_node_base& operator=( const linked_list_node_base& ) = delete;

        linked_list_node_base( linked_list_node_base&& other ) noexcept
        {
                *this = std::move( other );
        }

        linked_list_node_base& operator=( linked_list_node_base&& other ) noexcept
        {
                if ( &other == this ) {
                        return *this;
                }

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

        void link_as_next( linked_list_node_base< Base >& node )
        {
                node.next_ = next_;
                if ( next_ != nullptr ) {
                        next_->prev_ = &node;
                }

                node.prev_ = this;
                next_      = &node;
        }

        virtual Base&       operator*()       = 0;
        virtual const Base& operator*() const = 0;

        virtual Base*       operator->()       = 0;
        virtual const Base* operator->() const = 0;

        [[nodiscard]] linked_list_node_base* get_next()
        {
                return next_;
        }

        [[nodiscard]] linked_list_node_base* get_next( std::size_t id )
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

        [[nodiscard]] linked_list_node_base* get_prev()
        {
                return prev_;
        }

        [[nodiscard]] linked_list_node_base* get_prev( const std::size_t id )
        {
                if ( id == 0 ) {
                        return this;
                }
                if ( prev_ == nullptr ) {
                        return nullptr;
                }
                return prev_->get_prev( id - 1 );
        }

        virtual ~linked_list_node_base()
        {
                if ( prev_ != nullptr ) {
                        prev_->next_ = next_;
                }
                if ( next_ != nullptr ) {
                        next_->prev_ = prev_;
                }
        }

private:
        linked_list_node_base* next_ = nullptr;
        linked_list_node_base* prev_ = nullptr;
};

template < typename T, typename Base >
class linked_list_node : public linked_list_node_base< Base >
{
public:
        linked_list_node() = default;

        template < typename... Args >
        explicit linked_list_node( Args&&... args )
          : item_( std::forward< Args >( args )... )
        {
        }

        T& get()
        {
                return item_;
        }

        const T& get() const
        {
                return item_;
        }

        Base& operator*() override
        {
                return item_;
        }

        const Base& operator*() const override
        {
                return item_;
        }

        Base* operator->() override
        {
                return &item_;
        }

        const Base* operator->() const override
        {
                return &item_;
        }

        linked_list_node( linked_list_node&& other ) noexcept = default;

        linked_list_node& operator=( linked_list_node&& other ) noexcept = default;

private:
        T item_;
};

}  // namespace emlabcpp
