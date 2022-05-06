#include "emlabcpp/allocator/util.h"

#include <functional>
#include <memory>

#pragma once

namespace emlabcpp
{

namespace detail
{
        template < typename ReturnType, typename... ArgTypes >
        class static_function_storage_interface
        {
        public:
                virtual static_function_storage_interface* move_to( void* )      = 0;
                virtual static_function_storage_interface* copy_to( void* )      = 0;
                virtual ReturnType                         invoke( ArgTypes... ) = 0;
                virtual ~static_function_storage_interface()                     = default;
        };

        template < typename T, typename ReturnType, typename... ArgTypes >
        class static_function_storage final
          : public static_function_storage_interface< ReturnType, ArgTypes... >
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

                // TODO: function that tells size of T including alignment given ptr
                static_function_storage_interface< ReturnType, ArgTypes... >* move_to( void* ptr )
                {
                        ptr = align( ptr, alignof( static_function_storage ) );
                        // TODO: alignment
                        return std::construct_at(
                            reinterpret_cast< static_function_storage* >( ptr ),
                            std::move( item_ ) );
                }

                static_function_storage_interface< ReturnType, ArgTypes... >* copy_to( void* ptr )
                {
                        ptr = align( ptr, alignof( static_function_storage ) );
                        // TODO: alignment
                        return std::construct_at(
                            reinterpret_cast< static_function_storage* >( ptr ), item_ );
                }

                ReturnType invoke( ArgTypes... args )
                {
                        return std::invoke( item_, std::forward< ArgTypes >( args )... );
                }

                ~static_function_storage() = default;
        };
}  // namespace detail

template < typename CallableType, std::size_t Capacity >
class static_function;

template < typename ReturnType, typename... ArgTypes, std::size_t Capacity >
class static_function< ReturnType( ArgTypes... ), Capacity >
{
public:
        // TODO: maybe storage should also count the size of 'static_function_storage' ?
        // that is: only sizeof(T) bytes from storage should be used for T, not
        // sizeof(static_function_storage<T>) bytes
        using storage_type = std::byte[Capacity];
        using result_type  = ReturnType;

        static_function()
          : static_function( nullptr )
        {
        }

        static_function( std::nullptr_t )
          : interface_( nullptr )
        {
        }

        static_function( const static_function& other )
        {
                if ( other ) {
                        interface_ = other.interface_->copy_to( &storage_ );
                }
        }

        static_function( static_function&& other )
        {
                if ( other ) {
                        interface_ = other.interface_->move_to( &storage_ );
                }
        }

        template < typename Callable >
        static_function( Callable c )
        {
                // TODO: alignment
                // TODO: check that class fits with alignment into storage
                interface_ = std::construct_at(
                    reinterpret_cast<
                        detail::static_function_storage< Callable, ReturnType, ArgTypes... >* >(
                        &storage_ ),
                    std::move( c ) );
        }

        static_function& operator=( const static_function& other )
        {
                if ( this == &other ) {
                        return *this;
                }

                clear();

                if ( other ) {
                        interface_ = other.interface_->copy_to( &storage_ );
                }

                return *this;
        }

        static_function& operator=( static_function&& other )
        {
                if ( this == &other ) {
                        return *this;
                }

                clear();

                if ( other ) {
                        interface_ = other.interface_->move_to( &storage_ );  // TODO alignment
                }

                return *this;
        }

        static_function& operator=( std::nullptr_t )
        {
                clear();
                return *this;
        }

        template < typename Callable >
        static_function& operator=( Callable c )
        {
                clear();
                interface_ = std::construct_at(
                    reinterpret_cast<
                        detail::static_function_storage< Callable, ReturnType, ArgTypes... >* >(
                        &storage_ ),
                    std::move( c ) );
        }

        operator bool() const noexcept
        {
                return interface_ != nullptr;
        }

        ReturnType operator()( ArgTypes... args )
        {
                return interface_->invoke( std::forward< ArgTypes >( args )... );
        }

        ~static_function()
        {
                clear();
        }

private:
        void clear()
        {
                if ( interface_ != nullptr ) {
                        std::destroy_at( interface_ );
                        interface_ = nullptr;
                }
        }

        detail::static_function_storage_interface< ReturnType, ArgTypes... >* interface_;
        storage_type                                                          storage_;
};

}  // namespace emlabcpp
