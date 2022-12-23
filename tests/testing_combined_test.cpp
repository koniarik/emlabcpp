#include "test.h"

#include "emlabcpp/experimental/testing/controller.h"
#include "emlabcpp/experimental/testing/reactor.h"
#include "emlabcpp/pmr/new_delete_resource.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

struct reactor_iface : testing::reactor_interface
{
        testing::controller* cont = nullptr;

        void transmit( std::span< uint8_t > data )
        {
                cont->on_msg( data );
        }
};

struct controller_iface : testing::controller_interface
{
        testing::reactor* reac = nullptr;

        void transmit( std::span< uint8_t > data )
        {
                reac->on_msg( data );
        }
        void on_result( const testing::test_result& )
        {
        }
        void on_error( const testing::error_variant& )
        {
        }
};

TEST( testing_combined, base )
{
        reactor_iface    riface;
        controller_iface ciface;

        testing::reactor reac{ "reac", riface };
        ciface.reac = &reac;

        testing::controller cont{ pmr::new_delete_resource(), ciface };
        riface.cont = &cont;

        simple_test_fixture tf{ "test" };
        reac.register_test( tf );

        while ( cont.is_initializing() ) {
                reac.tick();
                cont.tick();
        }

        cont.start_test( 0 );

        while ( cont.is_test_running() ) {
                reac.tick();
                cont.tick();
        }

        EXPECT_EQ( tf.setup_count, 1 );
        EXPECT_EQ( tf.run_count, 1 );
        EXPECT_EQ( tf.teardown_count, 1 );
}

}  // namespace emlabcpp
