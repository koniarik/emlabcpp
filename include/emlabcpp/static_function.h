// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//
//  Copyright © 2022 Jan Veverak Koniarik
//  This file is part of project: emlabcpp
//
#include "emlabcpp/allocator/util.h"

#include <functional>
#include <memory>

#pragma once

namespace emlabcpp
{

namespace detail
{
        enum class static_function_operations
        {
                COPY,
                MOVE,
                DESTROY
        };

        template < typename ReturnType, typename... ArgTypes >
        struct static_function_vtable
        {
                ReturnType ( *invoke )( void*, ArgTypes... );
                void* ( *handle )( void*, void*, static_function_operations );
        };

        template < typename T, typename ReturnType, typename... ArgTypes >
        class static_function_storage
        {
                [[no_unique_address]] T item_;

        public:
                explicit static_function_storage( T&& item )
                  : item_( std::move( item ) )
                {
                }

                explicit static_function_storage( const T& item )
                  : item_( item )
                {
                }

                static void* construct_at( void* ptr, T item )
                {
                        ptr = align( ptr, alignof( static_function_storage ) );
                        return std::construct_at(
                            reinterpret_cast< static_function_storage* >( ptr ),
                            std::move( item ) );
                }

                static ReturnType invoke( void* source, ArgTypes... args )
                {
                        auto ptr = reinterpret_cast< static_function_storage* >( source );
                        return std::invoke( ptr->item_, std::forward< ArgTypes >( args )... );
                }

                static void*
                handle( void* const source, void* target, const static_function_operations op )
                {
                        auto ptr = reinterpret_cast< static_function_storage* >( source );

                        if ( op == static_function_operations::DESTROY ) {
                                std::destroy_at( ptr );
                                return nullptr;
                        }

                        target = align( target, alignof( static_function_storage ) );

                        if ( op == static_function_operations::COPY ) {
                                return std::construct_at(
                                    reinterpret_cast< static_function_storage* >( target ), *ptr );
                        } else {  // static_function_operations::MOVE
                                return std::construct_at(
                                    reinterpret_cast< static_function_storage* >( target ),
                                    std::move( *ptr ) );
                        }
                }

                static constexpr static_function_vtable< ReturnType, ArgTypes... > vtable = {
                    invoke,
                    handle };
        };
}  // namespace detail

template < typename CallableType, std::size_t Capacity, std::size_t Align >
class static_function_base;

template < typename ReturnType, typename... ArgTypes, std::size_t Capacity, std::size_t Align >
class static_function_base< ReturnType( ArgTypes... ), Capacity, Align >
{
        using vtable = detail::static_function_vtable< ReturnType, ArgTypes... >;

public:
        // TODO: maybe storage should also count the size of 'static_function_storage' ?
        // that is: only sizeof(T) bytes from storage should be used for T, not
        // sizeof(static_function_storage<T>) bytes
        using storage_type = std::byte[Capacity];
        using result_type  = ReturnType;

        static_function_base() = default;

        explicit static_function_base( const std::nullptr_t )
          : static_function_base()
        {
        }

        static_function_base( const static_function_base& other )
        {
                *this = other;
        }

        static_function_base( static_function_base&& other ) noexcept
        {
                *this = std::move( other );
        }

        template < typename Callable >
        explicit( false ) static_function_base( Callable c )
        {
                *this = std::move( c );
        }

        static_function_base& operator=( const static_function_base& other )
        {
                if ( this == &other ) {
                        return *this;
                }

                clear();

                if ( other ) {
                        obj_ = other.vtable_->handle(
                            other.obj_, &storage_, detail::static_function_operations::COPY );
                        vtable_ = other.vtable_;
                }

                return *this;
        }

        static_function_base& operator=( static_function_base&& other ) noexcept
        {
                if ( this == &other ) {
                        return *this;
                }

                clear();

                if ( other ) {
                        obj_ = other.vtable_->handle(
                            other.obj_, &storage_, detail::static_function_operations::MOVE );
                        vtable_ = other.vtable_;
                }

                return *this;
        }

        static_function_base& operator=( const std::nullptr_t )
        {
                clear();
                return *this;
        }

        template < typename Callable >
        static_function_base& operator=( Callable c )
        {
                clear();

                using storage =
                    detail::static_function_storage< Callable, ReturnType, ArgTypes... >;

                static constexpr std::size_t requires_space =
                    required_space( sizeof( storage ), alignof( storage ) );

                static_assert(
                    requires_space <= Capacity, "Callable would not fit into the static_function" );

                // TODO: check that class fits with alignment into storage
                obj_    = storage::construct_at( &storage_, std::move( c ) );
                vtable_ = &storage::vtable;

                return *this;
        }

        explicit( false ) operator bool() const noexcept
        {
                return obj_ != nullptr;
        }

        ReturnType operator()( ArgTypes... args )
        {
                return vtable_->invoke( obj_, std::forward< ArgTypes >( args )... );
        }

        ~static_function_base()
        {
                clear();
        }

private:
        static constexpr std::size_t required_space( std::size_t size, const std::size_t align )
        {
                if ( align > Align ) {
                        size += align - Align;
                }
                return size;
        }

        void clear()
        {
                if ( obj_ != nullptr ) {
                        // temporary obj required for throwing destructors
                        void*         obj  = obj_;
                        const vtable* vtbl = vtable_;
                        obj_               = nullptr;
                        vtable_            = nullptr;
                        vtbl->handle( obj, nullptr, detail::static_function_operations::DESTROY );
                }
        }

        const vtable* vtable_ = nullptr;
        void*         obj_    = nullptr;
        alignas( Align ) storage_type storage_;
};

template < typename Signature, std::size_t Capacity >
using static_function = static_function_base< Signature, Capacity, alignof( void* ) >;

}  // namespace emlabcpp
