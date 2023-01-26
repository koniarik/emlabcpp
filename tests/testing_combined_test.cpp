#include "test.h"

#include "emlabcpp/experimental/multiplexer.h"
#include "emlabcpp/experimental/testing/collect.h"
#include "emlabcpp/experimental/testing/controller.h"
#include "emlabcpp/experimental/testing/parameters.h"
#include "emlabcpp/experimental/testing/reactor.h"
#include "emlabcpp/pmr/new_delete_resource.h"

#include <gtest/gtest.h>

namespace emlabcpp
{

struct controller_iface : testing::controller_interface
{
        void on_result( const testing::test_result& )
        {
        }
        void on_error( const testing::error_variant& )
        {
        }
};

TEST( testing_combined, base )
{
        testing::controller* con_ptr;
        testing::reactor*    reac_ptr;
        auto                 reactor_send_f = [&]( auto, const auto& data ) {
                con_ptr->on_msg( data );
        };
        auto contr_send_f = [&]( auto, const auto& data ) {
                reac_ptr->on_msg( data );
        };

        testing::reactor reac{ 0, "reac", reactor_send_f };
        reac_ptr = &reac;

        controller_iface    ciface;
        testing::controller cont{ 0, pmr::new_delete_resource(), ciface, contr_send_f };
        con_ptr = &cont;

        testing::test_unit< simple_test_fixture > tf{ reac };

        while ( cont.is_initializing() ) {
                reac.tick();
                cont.tick();
        }

        cont.start_test( 0 );

        while ( cont.is_test_running() ) {
                reac.tick();
                cont.tick();
        }

        EXPECT_EQ( tf->setup_count, 1 );
        EXPECT_EQ( tf->run_count, 1 );
        EXPECT_EQ( tf->teardown_count, 1 );
}

struct complex_controller_iface : testing::controller_interface
{
        void on_result( const testing::test_result& )
        {
        }
        void on_error( const testing::error_variant& )
        {
        }
};

struct host_items
{
        complex_controller_iface ciface{};
        testing::controller      cont{
            testing::core_channel,
            pmr::new_delete_resource(),
            ciface,
            [&]( auto chan, const auto& data ) {
                    send( chan, data );
            } };
        testing::collect_server col_serv{
            testing::collect_channel,
            pmr::new_delete_resource(),
            [&]( auto chan, const auto& data ) {
                    send( chan, data );
            } };
        testing::parameters_server param_serv{
            testing::params_channel,
            testing::data_tree{ pmr::new_delete_resource() },
            [&]( auto chan, const auto& data ) {
                    send( chan, data );
            } };

        std::function< void( std::span< const uint8_t > ) > cb;
        testing::endpoint                                   ep;

        template < std::size_t N >
        void send( protocol::channel_type channel, const protocol::message< N >& data )
        {
                cb( ep.serialize( channel, data ) );
        }

        void on_msg( const std::span< const uint8_t > data )
        {
                ep.insert( data );
                ep.dispatch_value( cont, col_serv, param_serv );
        }
};

struct dev_items
{
        testing::reactor   reac{ testing::core_channel, "reac", [&]( auto chan, const auto& data ) {
                                      send( chan, data );
                              } };
        testing::collector coll{ testing::collect_channel, [&]( auto chan, const auto& data ) {
                                        send( chan, data );
                                } };
        testing::parameters params{ testing::params_channel, [&]( auto chan, const auto& data ) {
                                           send( chan, data );
                                   } };

        std::function< void( std::span< const uint8_t > ) > cb;
        testing::endpoint                                   ep;

        template < std::size_t N >
        void send( protocol::channel_type channel, const protocol::message< N >& data )
        {
                cb( ep.serialize( channel, data ) );
        }

        void on_msg( const std::span< const uint8_t > data )
        {
                ep.insert( data );
                ep.dispatch_value( reac, coll, params );
        }
};

TEST( testing_combined, complex )
{

        host_items host;
        dev_items  dev;

        host.cb = [&]( auto data ) {
                EMLABCPP_LOG( "to dev: " << data );
                dev.on_msg( data );
        };
        dev.cb = [&]( auto data ) {
                EMLABCPP_LOG( "to host: " << data );
                host.on_msg( data );
        };

        testing::test_unit< simple_test_fixture > tf{ dev.reac };

        while ( host.cont.is_initializing() ) {
                dev.reac.tick();
                host.cont.tick();
        }

        host.cont.start_test( 0 );

        while ( host.cont.is_test_running() ) {
                dev.reac.tick();
                host.cont.tick();
        }
}

}  // namespace emlabcpp
