#include "emlabcpp/allocator/pool.h"
#include "emlabcpp/assert.h"
#include "emlabcpp/defer.h"
#include "emlabcpp/experimental/testing/interface.h"
#include "emlabcpp/experimental/testing/protocol.h"
#include "emlabcpp/experimental/testing/reactor_interface.h"
#include "emlabcpp/protocol/packet_handler.h"
#include "emlabcpp/view.h"
#include "emlabcpp/visit.h"

#include <list>

#pragma once

namespace emlabcpp
{

class testing_reactor
{

        struct test_handle
        {
                std::string_view   name;
                testing_interface* ptr;
        };

        using handle_container = pool_list< test_handle >;
        using handle_iterator  = typename handle_container::iterator;

        std::string_view suite_name_;
        std::string_view suite_date_ = __DATE__;
        handle_container handles_;
        pool_interface*  mem_;

        struct active_execution
        {
                testing_test_id tid;
                testing_run_id  rid;
                test_handle&    handle;
        };

        std::optional< active_execution > active_exec_;

public:
        testing_reactor( std::string_view suite_name, pool_interface* mem )
          : suite_name_( suite_name )
          , handles_( mem )
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
        void handle_message( tag< TESTING_SUITE_NAME >, testing_reactor_interface_adapter& );
        void handle_message( tag< TESTING_SUITE_DATE >, testing_reactor_interface_adapter& );
        void handle_message( tag< TESTING_COUNT >, testing_reactor_interface_adapter& );
        void handle_message(
            tag< TESTING_NAME >,
            testing_test_id tid,
            testing_reactor_interface_adapter& );
        void handle_message(
            tag< TESTING_LOAD >,
            testing_test_id tid,
            testing_run_id  rid,
            testing_reactor_interface_adapter& );
        void handle_message(
            tag< TESTING_ARG >,
            testing_run_id,
            testing_key,
            testing_arg_variant,
            testing_reactor_interface_adapter& );
        void handle_message(
            tag< TESTING_ARG_MISSING >,
            testing_run_id,
            testing_key,
            testing_reactor_interface_adapter& );
        void
        handle_message( tag< TESTING_EXEC >, testing_run_id, testing_reactor_interface_adapter& );

        template < testing_test T >
        void store_test( std::string_view name, T t )
        {
                void* target = mem_->allocate( sizeof( T ) );
                T*    obj = std::construct_at( reinterpret_cast< T* >( target ), std::move( t ) );
                handles_.emplace_back( name, obj );
        }

        static void test_deleter( pool_interface& mem, void* p )
        {
                mem.deallocate( p );
        }

        void exec_test( testing_reactor_interface_adapter& comm );

        test_handle& access_test( testing_test_id );

public:
        ~testing_reactor();
};

class testing_default_reactor : private pool_base< 48, 32 >, public testing_reactor
{
public:
        // TODO: this may not be the best idea, as pool_mem will exist only _after_ constructor
        // for base class is called
        testing_default_reactor( std::string_view name )
          : testing_reactor( name, &this->pool_memory )
        {
        }
};

}  // namespace emlabcpp
