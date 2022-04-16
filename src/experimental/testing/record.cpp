#include "emlabcpp/experimental/testing/record.h"

#include "experimental/testing/reactor_interface_adapter.h"

using namespace emlabcpp;

testing_arg_variant testing_record::get_arg( testing_key key )
{
        comm_.reply< TESTING_ARG >( rid_, key );

        std::optional< testing_arg_variant > res;

        std::optional< testing_controller_reactor_variant > opt_var = comm_.read_variant();
        EMLABCPP_ASSERT( opt_var );
        apply_on_match(
            *opt_var,
            [&]( tag< TESTING_ARG >, testing_run_id, testing_key, testing_arg_variant var ) {
                    res = var;
            },
            [&]( tag< TESTING_ARG_MISSING >, testing_run_id, testing_key ) {
                    // TODO: error handling
            },
            [&]< auto ID >( tag< ID >, auto... ){
                // TODO: add error handling
            } );
        EMLABCPP_ASSERT( res );
        return *res;
}

void testing_record::collect( testing_key key, testing_arg_variant arg )
{
        comm_.reply< TESTING_COLLECT >( rid_, key, arg );
}
