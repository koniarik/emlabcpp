#include "emlabcpp/allocator/pool.h"
#include "emlabcpp/experimental/testing/controller_interface.h"
#include "emlabcpp/iterators/convert.h"
#include "emlabcpp/match.h"

#include <memory_resource>

#pragma once

namespace emlabcpp
{

class testing_controller_interface_adapter;

class testing_controller
{
public:
        struct test_info
        {
                testing_name_buffer name;
        };

        static std::optional< testing_controller >
        make( testing_controller_interface& iface, pool_interface* );

        [[nodiscard]] std::string_view suite_name() const
        {
                return { name_.begin(), name_.end() };
        }

        [[nodiscard]] std::string_view suite_date() const
        {
                return { date_.begin(), date_.end() };
        }

        [[nodiscard]] bool has_active_test() const
        {
                return context_.has_value();
        }

        [[nodiscard]] const pool_map< testing_test_id, test_info >& get_tests() const
        {
                return tests_;
        }

        void start_test( testing_test_id tid, testing_controller_interface& iface );

        void tick( testing_controller_interface& iface );

private:
        pool_map< testing_test_id, test_info > tests_;
        testing_name_buffer                    name_;
        testing_name_buffer                    date_;
        std::optional< testing_result >        context_;
        testing_run_id                         rid_ = 0;

        testing_controller(
            testing_name_buffer                    name,
            testing_name_buffer                    date,
            pool_map< testing_test_id, test_info > tests )
          : tests_( std::move( tests ) )
          , name_( std::move( name ) )
          , date_( std::move( date ) )
        {
        }

        void
        handle_message( tag< TESTING_COUNT >, auto, testing_controller_interface_adapter iface );
        void
        handle_message( tag< TESTING_NAME >, auto, testing_controller_interface_adapter iface );
        void handle_message(
            tag< TESTING_ARG >,
            testing_run_id                       rid,
            testing_key                          k,
            testing_controller_interface_adapter iface );

        void handle_message(
            tag< TESTING_COLLECT >,
            testing_run_id             rid,
            const testing_key&         key,
            const testing_arg_variant& avar,
            testing_controller_interface_adapter );
        void
        handle_message( tag< TESTING_FINISHED >, auto, testing_controller_interface_adapter iface );
        void handle_message( tag< TESTING_ERROR >, auto, testing_controller_interface_adapter );
        void handle_message( tag< TESTING_FAILURE >, auto, testing_controller_interface_adapter );
        void handle_message(
            tag< TESTING_SUITE_NAME >,
            auto,
            testing_controller_interface_adapter iface );
        void handle_message(
            tag< TESTING_SUITE_DATE >,
            auto,
            testing_controller_interface_adapter iface );
        void handle_message(
            tag< TESTING_INTERNAL_ERROR >,
            testing_reactor_error_variant        err,
            testing_controller_interface_adapter iface );
        void handle_message(
            tag< TESTING_PROTOCOL_ERROR >,
            protocol_error_record                rec,
            testing_controller_interface_adapter iface );
};

}  // namespace emlabcpp
