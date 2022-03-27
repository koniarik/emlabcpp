#include "emlabcpp/assert.h"
#include "emlabcpp/defer.h"
#include "emlabcpp/experimental/testing/interface.h"
#include "emlabcpp/experimental/testing/protocol.h"
#include "emlabcpp/memory/bucket_resource.h"
#include "emlabcpp/protocol/packet_handler.h"
#include "emlabcpp/view.h"
#include "emlabcpp/visit.h"

#include <list>

#pragma once

namespace emlabcpp
{

class testing_reactor
{

        using test_deleter_ptr = void ( * )( std::pmr::memory_resource&, void* );

        struct test_handle
        {
                std::string_view   name;
                testing_interface* ptr;
                test_deleter_ptr   del_ptr;
        };

        using handle_container = std::pmr::list< test_handle >;
        using handle_iterator  = typename handle_container::iterator;

        std::string_view           suite_name_;
        std::string_view           suite_date_ = __DATE__;
        handle_container           handles_;
        std::pmr::memory_resource* mem_;

        struct active_execution
        {
                testing_test_id tid;
                testing_run_id  rid;
                test_handle&    handle;
        };

        std::optional< active_execution > active_exec_;

public:
        testing_reactor( std::string_view suite_name, std::pmr::memory_resource* mem )
          : suite_name_( suite_name )
          , mem_( mem )
        {
        }

        testing_reactor( const testing_reactor& ) = delete;
        testing_reactor( testing_reactor&& )      = delete;
        testing_reactor& operator=( const testing_reactor& ) = delete;
        testing_reactor& operator=( testing_reactor&& ) = delete;

        template < testing_test T >
        void register_test( std::string_view name, T t )
        {
                store_test( name, std::move( t ) );
        }

        template < testing_callable T >
        void register_callable( std::string_view name, T cb )
        {
                store_test( name, testing_callable_overlay{ std::move( cb ) } );
        }

        void spin( testing_reactor_interface& comm );

private:
        void handle_message( tag< TESTING_SUITE_NAME >, testing_reactor_interface& );
        void handle_message( tag< TESTING_SUITE_DATE >, testing_reactor_interface& );
        void handle_message( tag< TESTING_COUNT >, testing_reactor_interface& );
        void handle_message( tag< TESTING_NAME >, testing_test_id tid, testing_reactor_interface& );
        void handle_message(
            tag< TESTING_LOAD >,
            testing_test_id tid,
            testing_run_id  rid,
            testing_reactor_interface& );
        void handle_message(
            tag< TESTING_ARG >,
            testing_run_id,
            testing_key,
            testing_arg_variant,
            testing_reactor_interface& );
        void handle_message(
            tag< TESTING_ARG_MISSING >,
            testing_run_id,
            testing_key,
            testing_reactor_interface& );
        void handle_message( tag< TESTING_EXEC >, testing_run_id, testing_reactor_interface& );

        template < testing_test T >
        void store_test( std::string_view name, T t )
        {
                void* target = mem_->allocate( sizeof( T ), alignof( T ) );
                T*    obj = std::construct_at( reinterpret_cast< T* >( target ), std::move( t ) );
                handles_.emplace_back( name, obj, test_deleter< T > );
        }

        template < testing_test T >
        static void test_deleter( std::pmr::memory_resource& mem, void* p )
        {
                mem.deallocate( p, sizeof( T ), alignof( T ) );
        }

        void exec_test( testing_reactor_interface& comm );

        test_handle& access_test( testing_test_id );

public:
        ~testing_reactor();
};

class testing_default_reactor : public testing_reactor
{
        using mem_type = bucket_memory_resource< 32, 32 >;
        mem_type bucket_mem_;

public:
        // TODO: this may not be the best idea, as bucket_mem will exist only _after_ constructor
        // for base class is called
        testing_default_reactor( std::string_view name )
          : testing_reactor( name, &bucket_mem_ )
        {
        }
};

}  // namespace emlabcpp
