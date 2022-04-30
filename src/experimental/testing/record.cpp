#include "emlabcpp/experimental/testing/record.h"

#include "experimental/testing/reactor_interface_adapter.h"

using namespace emlabcpp;

std::optional< testing_arg_variant > testing_record::get_arg_variant( const testing_key& key )
{
        comm_.reply< TESTING_ARG >( rid_, key );

        std::optional< testing_arg_variant > res;

        std::optional< testing_controller_reactor_variant > opt_var = comm_.read_variant();
        if ( !opt_var ) {
                comm_.report_failure< TESTING_NO_RESPONSE_E >( TESTING_ARG );
                return {};
        }

        apply_on_match(
            *opt_var,
            [&]( tag< TESTING_ARG >,
                 testing_run_id,
                 const testing_key&,
                 const testing_arg_variant& var ) {
                    res = var;
            },
            [&]( tag< TESTING_ARG_MISSING >, testing_run_id, const testing_key& key ) {
                    comm_.report_failure< TESTING_ARG_MISSING_E >( key );
            },
            [&]< auto ID >( tag< ID >, auto... ) {
                    comm_.report_failure< TESTING_ARG_WRONG_MESSAGE_E >( ID );
            } );

        return res;
}

void testing_record::collect( const testing_key& key, const testing_arg_variant& arg )
{
        comm_.reply< TESTING_COLLECT >( rid_, key, arg );
}

void testing_record::report_wrong_type_error( const testing_key& key )
{
        comm_.report_failure< TESTING_ARG_WRONG_TYPE_E >( key );
}
