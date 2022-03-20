#include "emlabcpp/assert.h"
#include "emlabcpp/defer.h"
#include "emlabcpp/experimental/testing/interface.h"
#include "emlabcpp/experimental/testing/protocol.h"
#include "emlabcpp/memory/bucket_resource.h"
#include "emlabcpp/protocol/packet_handler.h"
#include "emlabcpp/view.h"
#include "emlabcpp/visit.h"

#pragma once

namespace emlabcpp
{

template < std::size_t TestCount >
class testing_reactor
{

        using test_deleter_ptr = void ( * )( std::pmr::memory_resource&, void* );

        struct test_handle
        {
                std::string_view   name;
                testing_interface* ptr;
                test_deleter_ptr   del_ptr;
        };

        using handle_container = static_vector< test_handle, TestCount >;
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

        void spin( testing_reactor_interface& comm )
        {
                auto opt_var = comm.read_variant();

                if ( !opt_var ) {
                        return;
                }

                apply_on_visit(
                    [&]( auto... args ) {
                            handle_message( args..., comm );
                    },
                    *opt_var );
        }

private:
        void handle_message( tag< TESTING_SUITE_NAME >, testing_reactor_interface& comm )
        {
                comm.reply< TESTING_SUITE_NAME >( testing_name_to_buffer( suite_name_ ) );
        }
        void handle_message( tag< TESTING_SUITE_DATE >, testing_reactor_interface& comm )
        {
                comm.reply< TESTING_SUITE_DATE >( testing_name_to_buffer( suite_date_ ) );
        }
        void handle_message( tag< TESTING_COUNT >, testing_reactor_interface& comm )
        {
                comm.reply< TESTING_COUNT >( static_cast< testing_test_id >( handles_.size() ) );
        }
        void
        handle_message( tag< TESTING_NAME >, testing_test_id tid, testing_reactor_interface& comm )
        {
                if ( tid >= handles_.size() ) {
                        comm.report_failure( TESTING_BAD_TEST_ID_E );
                        return;
                }
                comm.reply< TESTING_NAME >( testing_name_to_buffer( handles_[tid].name ) );
        }
        void handle_message(
            tag< TESTING_LOAD >,
            testing_test_id            tid,
            testing_run_id             rid,
            testing_reactor_interface& comm )
        {
                if ( active_exec_ ) {
                        comm.report_failure( TESTING_TEST_ALREADY_LOADED_E );
                        return;
                }

                if ( tid >= handles_.size() ) {
                        comm.report_failure( TESTING_BAD_TEST_ID_E );
                        return;
                }

                active_exec_.emplace( tid, rid, handles_[tid] );
        }
        void handle_message(
            tag< TESTING_ARG >,
            testing_run_id,
            testing_key,
            testing_arg_variant,
            testing_reactor_interface& comm )
        {
                comm.report_failure( TESTING_UNDESIRED_MSG_E );
        }
        void handle_message(
            tag< TESTING_ARG_MISSING >,
            testing_run_id,
            testing_key,
            testing_reactor_interface& comm )
        {
                comm.report_failure( TESTING_UNDESIRED_MSG_E );
        }
        void handle_message( tag< TESTING_EXEC >, testing_run_id, testing_reactor_interface& comm )
        {
                if ( !active_exec_ ) {
                        comm.report_failure( TESTING_TEST_NOT_LOADED_E );
                        return;
                }

                exec_test( comm );
                active_exec_.reset();
        }

        template < testing_test T >
        void store_test( std::string_view name, T t )
        {
                EMLABCPP_ASSERT( !handles_.full() );

                void* target = mem_->allocate( sizeof( T ), alignof( T ) );
                T*    obj = std::construct_at( reinterpret_cast< T* >( target ), std::move( t ) );
                handles_.emplace_back( name, obj, test_deleter< T > );
        }

        template < testing_test T >
        static void test_deleter( std::pmr::memory_resource& mem, void* p )
        {
                mem.deallocate( p, sizeof( T ), alignof( T ) );
        }

        void exec_test( testing_reactor_interface& comm )
        {
                defer d = [&] {
                        comm.reply< TESTING_FINISHED >( active_exec_->rid );
                };

                test_handle& h = active_exec_->handle;

                testing_record rec{ active_exec_->tid, active_exec_->rid, comm };

                h.ptr->setup( rec );
                if ( rec.errored() ) {
                        comm.reply< TESTING_FAILURE >( active_exec_->rid );
                        return;
                }

                h.ptr->run( rec );
                if ( rec.errored() ) {
                        comm.reply< TESTING_ERROR >( active_exec_->rid );
                }

                h.ptr->teardown( rec );
                if ( rec.errored() ) {
                        comm.reply< TESTING_FAILURE >( active_exec_->rid );
                }
        }

public:
        ~testing_reactor()
        {
                for ( test_handle& h : handles_ ) {
                        std::destroy_at( h.ptr );
                        h.del_ptr( *mem_, h.ptr );
                }
        }
};

class testing_default_reactor : public testing_reactor< 32 >
{
        static constexpr std::size_t test_count = 32;

        using mem_type = bucket_memory_resource< test_count, 32 >;
        mem_type bucket_mem_;

public:
        // TODO: this may not be the best idea, as bucket_mem will exist only _after_ constructor
        // for base class is called
        testing_default_reactor( std::string_view name )
          : testing_reactor< 32 >( name, &bucket_mem_ )
        {
        }
};

}  // namespace emlabcpp
