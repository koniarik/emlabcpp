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

        template < typename T, typename ReturnType, typename... ArgTypes >
        class static_function_storage
        {
                T item_;

        public:
                static_function_storage( T&& item )
                  : item_( std::move( item ) )
                {
                }

                static_function_storage( const T& item )
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
                        static_function_storage* ptr =
                            reinterpret_cast< static_function_storage* >( source );
                        return ptr->item_( std::forward< ArgTypes >( args )... );
                }

                static void* handle( void* source, void* target, static_function_operations op )
                {
                        static_function_storage* ptr =
                            reinterpret_cast< static_function_storage* >( source );

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
        };
}  // namespace detail

template < typename CallableType, std::size_t Capacity, std::size_t Align, bool NoexceptMove >
class static_function_base;

template <
    typename ReturnType,
    typename... ArgTypes,
    std::size_t Capacity,
    std::size_t Align,
    bool        NoexceptMove >
class static_function_base< ReturnType( ArgTypes... ), Capacity, Align, NoexceptMove >
{
        using invoke_type  = ReturnType ( * )( void*, ArgTypes... args );
        using handler_type = void* (*) ( void*, void*, detail::static_function_operations );

public:
        // TODO: maybe storage should also count the size of 'static_function_storage' ?
        // that is: only sizeof(T) bytes from storage should be used for T, not
        // sizeof(static_function_storage<T>) bytes
        using storage_type = alignas( Align ) std::byte[Capacity];
        using result_type  = ReturnType;

        static_function_base()
        {
        }

        static_function_base( std::nullptr_t )
          : static_function_base()
        {
        }

        static_function_base( const static_function_base& other )
        {
                *this = other;
        }

        static_function_base( static_function_base&& other ) noexcept( NoexceptMove )
        {
                *this = std::move( other );
        }

        template < typename Callable >
        static_function_base( Callable c )
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
                        obj_ = other.handle_(
                            other.obj_, &storage_, detail::static_function_operations::COPY );
                        invoke_ = other.invoke_;
                        handle_ = other.handle_;
                }

                return *this;
        }

        static_function_base& operator=( static_function_base&& other ) noexcept( NoexceptMove )
        {
                if ( this == &other ) {
                        return *this;
                }

                clear();

                if ( other ) {
                        obj_ = other.handle_(
                            other.obj_, &storage_, detail::static_function_operations::MOVE );
                        invoke_ = other.invoke_;
                        handle_ = other.handle_;
                }

                return *this;
        }

        static_function_base& operator=( std::nullptr_t )
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

                if constexpr ( NoexceptMove ) {
                        static_assert(
                            std::is_nothrow_move_constructible_v< Callable > &&
                                std::is_nothrow_move_assignable_v< Callable >,
                            "Callable has to use nothrow moves if NoexceptMove enabled for static_function" );
                }

                // TODO: check that class fits with alignment into storage
                obj_    = storage::construct_at( &storage_, std::move( c ) );
                invoke_ = storage::invoke;
                handle_ = storage::handle;

                return *this;
        }

        operator bool() const noexcept
        {
                return obj_ != nullptr;
        }

        ReturnType operator()( ArgTypes... args )
        {
                return invoke_( obj_, std::forward< ArgTypes >( args )... );
        }

        ~static_function_base()
        {
                clear();
        }

private:

        static constexpr std::size_t required_space( std::size_t size, std::size_t align )
        {
                if ( align > Align ) {
                        size += align - Align;
                }
                return size;
        }

        void clear()
        {
                if ( obj_ != nullptr ) {
                        handle_( obj_, nullptr, detail::static_function_operations::DESTROY );
                        obj_    = nullptr;
                        invoke_ = nullptr;
                        handle_ = nullptr;
                }
        }

        invoke_type  invoke_ = nullptr;
        handler_type handle_ = nullptr;
        void*        obj_    = nullptr;
        storage_type storage_;
};

template < typename Signature, std::size_t Capacity >
using static_function = static_function_base< Signature, Capacity, alignof( void* ), false >;

}  // namespace emlabcpp
