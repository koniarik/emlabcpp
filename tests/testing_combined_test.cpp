///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
/// and associated documentation files (the "Software"), to deal in the Software without
/// restriction, including without limitation the rights to use, copy, modify, merge, publish,
/// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or
/// substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
/// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
/// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

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
        void on_result( const testing::test_result& ) override
        {
        }

        void on_error( const testing::error_variant& ) override
        {
        }
};

TEST( testing_combined, base )
{
        testing::controller* con_ptr;
        testing::reactor*    reac_ptr;
        auto                 reactor_send_f = [&]( auto, const auto& data ) {
                return con_ptr->on_msg( data ).has_errored_result();
        };
        auto contr_send_f = [&]( auto, const auto& data ) {
                return reac_ptr->on_msg( data ).has_errored_result();
        };

        testing::reactor reac{ 0, "reac", reactor_send_f };
        reac_ptr = &reac;

        controller_iface    ciface;
        testing::controller cont{ 0, pmr::new_delete_resource(), ciface, contr_send_f };
        con_ptr = &cont;

        testing::test_unit< simple_test_fixture > tf{};
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

        EXPECT_EQ( tf.get().setup_count, 1 );
        EXPECT_EQ( tf.get().run_count, 1 );
        EXPECT_EQ( tf.get().teardown_count, 1 );
}

struct complex_controller_iface : testing::controller_interface
{
        void on_result( const testing::test_result& ) override
        {
        }

        void on_error( const testing::error_variant& ) override
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
                    return send( chan, data );
            } };
        testing::collect_server col_serv{
            testing::collect_channel,
            pmr::new_delete_resource(),
            [&]( auto chan, const auto& data ) {
                    return send( chan, data );
            } };
        testing::parameters_server param_serv{
            testing::params_channel,
            testing::data_tree{ pmr::new_delete_resource() },
            [&]( auto chan, const auto& data ) {
                    return send( chan, data );
            } };

        std::function< result( std::span< const std::byte > ) > cb;
        testing::endpoint                                       ep;

        template < std::size_t N >
        result send( protocol::channel_type channel, const protocol::message< N >& data )
        {
                return cb( ep.serialize( channel, data ) );
        }

        result on_msg( const std::span< const std::byte > data )
        {
                ep.insert( data );
                return ep.dispatch_value( cont, col_serv, param_serv ).has_errored_result();
        }
};

struct dev_items
{
        testing::reactor   reac{ testing::core_channel, "reac", [&]( auto chan, const auto& data ) {
                                      return send( chan, data );
                              } };
        testing::collector coll{ testing::collect_channel, [&]( auto chan, const auto& data ) {
                                        return send( chan, data );
                                } };
        testing::parameters params{ testing::params_channel, [&]( auto chan, const auto& data ) {
                                           return send( chan, data );
                                   } };

        std::function< result( std::span< const std::byte > ) > cb;
        testing::endpoint                                       ep;

        template < std::size_t N >
        result send( protocol::channel_type channel, const protocol::message< N >& data )
        {
                return cb( ep.serialize( channel, data ) );
        }

        result on_msg( const std::span< const std::byte > data )
        {
                ep.insert( data );
                return ep.dispatch_value( reac, coll, params ).has_errored_result();
        }
};

TEST( testing_combined, complex )
{

        host_items host;
        dev_items  dev;

        host.cb = [&]( auto data ) {
                return dev.on_msg( data );
        };
        dev.cb = [&]( auto data ) {
                return host.on_msg( data );
        };

        testing::test_unit< simple_test_fixture > tf{};
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
