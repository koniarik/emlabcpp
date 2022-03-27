#include "emlabcpp/experimental/testing/reactor.h"

using namespace emlabcpp;

void testing_reactor::spin( testing_reactor_interface& comm )
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

void testing_reactor::handle_message( tag< TESTING_SUITE_NAME >, testing_reactor_interface& comm )
{
        comm.reply< TESTING_SUITE_NAME >( testing_name_to_buffer( suite_name_ ) );
}
void testing_reactor::handle_message( tag< TESTING_SUITE_DATE >, testing_reactor_interface& comm )
{
        comm.reply< TESTING_SUITE_DATE >( testing_name_to_buffer( suite_date_ ) );
}
void testing_reactor::handle_message( tag< TESTING_COUNT >, testing_reactor_interface& comm )
{
        comm.reply< TESTING_COUNT >( static_cast< testing_test_id >( handles_.size() ) );
}
void testing_reactor::handle_message(
    tag< TESTING_NAME >,
    testing_test_id            tid,
    testing_reactor_interface& comm )
{
        if ( tid >= handles_.size() ) {
                comm.report_failure( TESTING_BAD_TEST_ID_E );
                return;
        }
        comm.reply< TESTING_NAME >( testing_name_to_buffer( access_test( tid ).name ) );
}
void testing_reactor::handle_message(
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

        active_exec_.emplace( tid, rid, access_test( tid ) );
}
void testing_reactor::handle_message(
    tag< TESTING_ARG >,
    testing_run_id,
    testing_key,
    testing_arg_variant,
    testing_reactor_interface& comm )
{
        comm.report_failure( TESTING_UNDESIRED_MSG_E );
}
void testing_reactor::handle_message(
    tag< TESTING_ARG_MISSING >,
    testing_run_id,
    testing_key,
    testing_reactor_interface& comm )
{
        comm.report_failure( TESTING_UNDESIRED_MSG_E );
}
void testing_reactor::handle_message(
    tag< TESTING_EXEC >,
    testing_run_id,
    testing_reactor_interface& comm )
{
        if ( !active_exec_ ) {
                comm.report_failure( TESTING_TEST_NOT_LOADED_E );
                return;
        }

        exec_test( comm );
        active_exec_.reset();
}

void testing_reactor::exec_test( testing_reactor_interface& comm )
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

testing_reactor::test_handle& testing_reactor::access_test( testing_test_id tid )
{
        auto iter = handles_.begin();
        std::advance( iter, tid );
        return *iter;
}

testing_reactor::~testing_reactor()
{
        for ( test_handle& h : handles_ ) {
                std::destroy_at( h.ptr );
                h.del_ptr( *mem_, h.ptr );
        }
}
