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
        testing::reactor* reac = nullptr;

        void transmit( const testing::controller_reactor_message& data )
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
        testing::controller* con_ptr;
        auto                 reactor_send_f = [&]( auto data ) {
                con_ptr->on_msg( data );
        };

        controller_iface ciface;

        testing::reactor reac{ "reac", reactor_send_f };
        ciface.reac = &reac;

        testing::controller cont{ pmr::new_delete_resource(), ciface };
        con_ptr = &cont;

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

struct complex_controller_iface : testing::controller_interface
{
        complex_controller_iface( testing::controller_transmit_callback cb )
          : cb_( std::move( cb ) )
        {
        }

        testing::controller_transmit_callback cb_;

        void transmit( const testing::controller_reactor_message& data )
        {
                cb_( data );
        }
        void on_result( const testing::test_result& )
        {
        }
        void on_error( const testing::error_variant& )
        {
        }
};

struct host_items
{
        complex_controller_iface   ciface{ [&]( const auto& data ) {
                send( 1, data );
        } };
        testing::controller        cont{ pmr::new_delete_resource(), ciface };
        testing::collect_server    col_serv{ pmr::new_delete_resource(), [&]( const auto& data ) {
                                                 send( 2, data );
                                         } };
        testing::parameters_server param_serv{
            testing::data_tree{ pmr::new_delete_resource() },
            [&]( const auto& data ) {
                    send( 3, data );
            } };

        std::function< void( std::span< const uint8_t > ) > cb;
        testing::endpoint                                   ep;

        template < std::size_t N >
        void send( protocol::port_type port, const protocol::message< N >& data )
        {
                cb( ep.serialize(
                    std::make_tuple( port, protocol::sizeless_message< 64 >{ data } ) ) );
        }

        void on_msg( const std::span< const uint8_t > data )
        {
                ep.insert( data );
                match(
                    ep.get_value(),
                    [&]( std::size_t ) {
                            FAIL();
                    },
                    [&]( const auto& payload ) {
                            const auto& [port, data] = payload;
                            switch ( port ) {
                                    case 1:
                                            cont.on_msg( data );
                                            break;
                                    case 2:
                                            col_serv.on_msg( data );
                                            break;
                                    case 3:
                                            param_serv.on_msg( data );
                                            break;
                            }
                    },
                    [&]( protocol::error_record e ) {
                            FAIL() << e;
                    } );
        }
};

struct dev_items
{
        testing::reactor    reac{ "reac", [&]( const auto& data ) {
                                      send( 1, data );
                              } };
        testing::collector  coll{ [&]( const auto& data ) {
                send( 2, data );
        } };
        testing::parameters params{ [&]( const auto& data ) {
                send( 3, data );
        } };

        std::function< void( std::span< const uint8_t > ) > cb;
        testing::endpoint                                   ep;

        template < std::size_t N >
        void send( protocol::port_type port, const protocol::message< N >& data )
        {
                cb( ep.serialize(
                    std::make_tuple( port, protocol::sizeless_message< 64 >{ data } ) ) );
        }

        void on_msg( const std::span< const uint8_t > data )
        {
                ep.insert( data );
                match(
                    ep.get_value(),
                    [&]( std::size_t ) {
                            FAIL();
                    },
                    [&]( const auto& payload ) {
                            const auto& [port, data] = payload;
                            switch ( port ) {
                                    case 1:
                                            reac.on_msg( data );
                                            break;
                                    case 2:
                                            coll.on_msg( data );
                                            break;
                                    case 3:
                                            params.on_msg( data );
                                            break;
                            }
                    },
                    [&]( protocol::error_record e ) {
                            FAIL() << e;
                    } );
        }
};

TEST( testing_combined, complex )
{

        host_items host;
        dev_items  dev;

        host.cb = [&]( auto data ) {
                EMLABCPP_LOG("to dev: " << data);
                dev.on_msg( data );
        };
        dev.cb = [&]( auto data ) {
                EMLABCPP_LOG("to host: " << data);
                host.on_msg( data );
        };

        simple_test_fixture tf{ "test" };
        dev.reac.register_test( tf );

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
