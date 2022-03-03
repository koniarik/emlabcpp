#include "emlabcpp/memory/bucket_resource.h"
#include "emlabcpp/testing/interface.h"

#pragma once

namespace emlabcpp
{

template < typename T >
concept testing_test = std::derived_from< T, testing_interface > && std::movable< T >;

template < typename T >
concept testing_callable = requires( T t, testing_record& rec )
{
        t( rec );
};

struct testing_default_config
{
        static constexpr std::size_t max_test_count       = 32;
        static constexpr std::size_t max_test_memory_size = 32;
};

template < typename Config >
class testing_reactor
{
        struct test_handle
        {
                std::string_view name;
                test_interface*  ptr;
        };

        em::static_vector< test_handle, max_test_count >                               handles_;
        bucket_memory_resource< Config::max_test_count, Config::max_test_memory_size > mem_;

public:
        template < typename T >
        void register_test( std::string_view, testing_test T )
        {
                store_test( name, std::move( T ) );
        }

        template < typename T >
        void register_callable( std::string_view name, testing_callable T cb )
        {
                store_test( name, testing_callable_overlay{ std::move( cb ) } );
        }

        void spin( std::span< uint8_t > data )
        {
        }

private:
        template < typename T >
        void store_test( std::string_view name, testing_callable T t )
        {
                SCHPIN_ASSERT( !handles_.full() );
                void* target = mem_->allocate( sizeof( T ), alignof( T ) );
                T*    obj = std::construct_at( reinterpret_cast< T* >( target ), std::move( t ) );
                handles_.emplace_back( test_handle{ name, obj } );
        }

        ~testing_reactor()
        {
                for ( test_handle& h : reversed( handles_ ) ) {
                        std::destroy_at( h.ptr );
                        mem_->deallocate( h.ptr );
                }
        }
};

}  // namespace emlabcpp
