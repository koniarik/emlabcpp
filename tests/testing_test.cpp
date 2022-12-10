// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//
//  Copyright © 2022 Jan Veverak Koniarik
//  This file is part of project: emlabcpp
//
#include "emlabcpp/experimental/logging.h"
#include "emlabcpp/experimental/testing/controller.h"
#include "emlabcpp/experimental/testing/gtest.h"
#include "emlabcpp/experimental/testing/json.h"
#include "emlabcpp/experimental/testing/reactor.h"
#include "emlabcpp/protocol/streams.h"
#include "util.h"

#include <gtest/gtest.h>

namespace em = emlabcpp;
using namespace std::literals;  // TODO: << get rid of this

// ----------------------------------------------------------------------------
// design:
//
//  reactor: object that exists on device with tests
//    - tests are registered into reactor
//    - reactor executes them based on communication with controller
//
//  controller: object that exists on orchestrating device
//    - asks reactor for list of tests
//    - can initiate execution of test
//
//  record: interface in tests
//    - an API passed for each test that serves as an interface into
//      the framework
//
//  reactor/controller_interface: base classes
//    - these interfaces are used by the objects to communicate between
//      each other
//    - in case of controller also reporting errors/test data

// ----------------------------------------------------------------------------
// tests definitions

em::testing::test_coroutine simple_test_case( em::pmr::memory_resource&, em::testing::record& rec )
{

        // request an argument from controller
        em::testing::node_id arg_id = co_await rec.get_param_child( 0, "simple_test" );
        int64_t              arg    = co_await rec.get_param< int64_t >( arg_id );

        std::ignore = arg;

        // collecting records data in the controller
        // this is stored for later review
        co_await rec.collect( 0, "col1", "some_data"sv );
        co_await rec.collect( 0, "33", "foooo"sv );

        // of course, we can decide whenever the test
        // failed or succeeded
        rec.fail();
        // rec.success()

        // just fail/success would pollute tests with
        // if(2 > 3){ fail(); }
        //
        // there is a shortand for that:
        rec.expect( 2 < 3 );
}

struct my_test_fixture : em::testing::test_interface
{
        // The basic test unit is actually "em::testing::test_interface"
        // and it's run() method.
        //
        // `simple_test_case` is just called in run() method of
        // lambda handler for the testing::test_interface
        //
        // Apart from run, the test can also implement
        // setup/teardown methods for preparation of the tests.
        // If this methods fail (via rec.fail()) that is taken
        // as critical failure and run() is not executed.
        //
        // This can lead to preparing one test "fixture"
        // that executes preparation common in other tests
        // which just implement the run() method.

        my_test_fixture( em::testing::reactor& reactor, std::string_view name )
          : em::testing::test_interface( reactor, name )
        {
        }

        // disabling copy should be allowed
        my_test_fixture( const my_test_fixture& )            = delete;
        my_test_fixture& operator=( const my_test_fixture& ) = delete;

        my_test_fixture( my_test_fixture&& )            = default;
        my_test_fixture& operator=( my_test_fixture&& ) = default;

        em::testing::test_coroutine
        setup( em::pmr::memory_resource&, em::testing::record& ) override
        {
                // setup i2c
                co_return;
        }

        em::testing::test_coroutine run( em::pmr::memory_resource&, em::testing::record& ) override
        {
                // empty overload
                co_return;
        }

        em::testing::test_coroutine
        teardown( em::pmr::memory_resource&, em::testing::record& ) override
        {
                // teardown i2c
                co_return;
        }
};

struct my_test_case : my_test_fixture
{
        // testing class using basic fixture for preparation
        // tadaaaaah!

        my_test_case( em::testing::reactor& reactor )
          : my_test_fixture( reactor, "my_test_case" )
        {
        }

        em::testing::test_coroutine
        run( em::pmr::memory_resource&, em::testing::record& rec ) override
        {
                co_await rec.collect( 0, "some key for collector", 42, false );

                rec.fail();

                co_return;
        }
};

// ----------------------------------------------------------------------------
// interfaces that has to be implemented for the framework objects

struct reactor_iface : em::testing::reactor_interface
{
        em::thread_safe_queue& con_reac_buff;
        em::thread_safe_queue& reac_con_buff;

        reactor_iface( em::thread_safe_queue& cr, em::thread_safe_queue& rc )
          : con_reac_buff( cr )
          , reac_con_buff( rc )
        {
        }

        void transmit( std::span< uint8_t > inpt ) final
        {
                reac_con_buff.insert( inpt );
        }

        std::optional< em::static_vector< uint8_t, 64 > > receive( uint8_t ) final
        {
                return con_reac_buff.pop();
        }
};

// TODO: an idea
//  - split the controll interface into controller/controller_com interface
//  - most of the API for controller can be implemented specifically for gtest
//  - only thing that remains is the "comm" api
//  - if separated, that abstraction is easily possible
struct controller_iface : em::testing::controller_interface
{
        em::thread_safe_queue& con_reac_buff;
        em::thread_safe_queue& reac_con_buff;

        typename em::testing::data_tree::pool_type< 42 > pool;
        em::testing::data_tree                           tree;

        controller_iface( em::thread_safe_queue& cr, em::thread_safe_queue& rc )
          : con_reac_buff( cr )
          , reac_con_buff( rc )
          , pool()
          , tree( pool )
        {
                // we need test that uses more compelx tree
                nlohmann::json j = { { "simple_test", 32 }, { "complex_lambda", 42 } };

                std::optional opt_tree = em::testing::json_to_data_tree( pool, j );
                if ( opt_tree ) {
                        tree = std::move( *opt_tree );
                } else {
                        std::cout << "failed to build tree" << std::endl;
                }
        }

        void transmit( std::span< uint8_t > inpt ) final
        {
                con_reac_buff.insert( inpt );
        }

        std::optional< em::static_vector< uint8_t, 64 > > receive( std::size_t ) final
        {
                return { reac_con_buff.pop() };
        }

        // Once test is finished, the controller will
        // call this method with test result.
        //
        // There is support for integration into gtests,
        // that prints all collected data in failure
        void on_result( const em::testing::test_result& res ) final
        {
                EXPECT_PRED_FORMAT1( em::testing::gtest_predicate, res );
        }

        em::testing::data_tree& get_param_tree() final
        {
                return tree;
        }

        void on_error( em::testing::error_variant err ) final
        {
                FAIL() << err;
        }
};

int main( int argc, char** argv )
{
        testing::InitGoogleTest( &argc, argv );

        // ----------------------------------------------------------------------------
        // register tests and examples of lambda tests

        em::testing::reactor reac{ "emlabcpp::testing" };

        em::testing::test_callable st{ reac, "simple test", simple_test_case };

        em::testing::test_callable slt{
            reac,
            "simple lambda test",
            [&]( em::pmr::memory_resource&,
                 em::testing::record& rec ) -> em::testing::test_coroutine {
                    rec.success();
                    co_return;
            } };

        my_test_case mtc{ reac };

        em::testing::test_callable clt{
            reac,
            "complex lambda test",
            [&]( em::pmr::memory_resource&,
                 em::testing::record& rec ) -> em::testing::test_coroutine {
                    uint64_t data = co_await rec.get_param< uint64_t >( 0 );
                    std::ignore   = data;
                    rec.success();
                    co_return;
            } };

        auto laf = em::testing::test_compose(
            my_test_fixture{ reac, "lambda and fixture" },
            [&]( em::pmr::memory_resource&,
                 em::testing::record& rec ) -> em::testing::test_coroutine {
                    rec.expect( 1 > 0 );
                    co_return;
            } );

        // ----------------------------------------------------------------------------
        // build the virtual example and run it

        em::thread_safe_queue con_reac_buff;
        em::thread_safe_queue reac_con_buff;

        std::atomic< bool > finished_ = false;

        std::thread t1{ [&] {
                reactor_iface             iface{ con_reac_buff, reac_con_buff };
                std::chrono::milliseconds t{ 10 };

                while ( !finished_ ) {
                        reac.spin( iface );
                        std::this_thread::sleep_for( t );
                }
        } };

        controller_iface ci{ con_reac_buff, reac_con_buff };

        em::testing::register_gtests( ci );

        auto res = RUN_ALL_TESTS();

        finished_ = true;
        t1.join();

        std::ignore = res;
        // TODO: temporary solution
        return 0;
}

// and all this produces something like this, notice the collected data:
// ➜ ./build/tests/testing_test --gtest_color=yes
// [==========] Running 5 tests from 1 test suite.
// [----------] Global test environment set-up.
// [----------] 5 tests from emlabcpp::testing
// [ RUN      ] emlabcpp::testing.simple test
// /home/veverak/Projects/emlabcpp/test/tests/testing_test.cpp:169: Failure
// Test produced a failure, stopping
// collected:
//  33 :	foooo
//  col1 :	some_data
//  col2 :	42
// [  FAILED  ] emlabcpp::testing.simple test (101 ms)
// [ RUN      ] emlabcpp::testing.simple lambda test
// [       OK ] emlabcpp::testing.simple lambda test (102 ms)
// [ RUN      ] emlabcpp::testing.simple struct test
// /home/veverak/Projects/emlabcpp/test/tests/testing_test.cpp:169: Failure
// Test produced a failure, stopping
// collected:
//  some key for col :	42
// [  FAILED  ] emlabcpp::testing.simple struct test (102 ms)
// [ RUN      ] emlabcpp::testing.complex lambda test
// /home/veverak/Projects/emlabcpp/test/tests/testing_test.cpp:169: Failure
// Test produced a failure, stopping
// [  FAILED  ] emlabcpp::testing.complex lambda test (101 ms)
// [ RUN      ] emlabcpp::testing.lambda amd fixture
// [       OK ] emlabcpp::testing.lambda amd fixture (102 ms)
// [----------] 5 tests from emlabcpp::testing (508 ms total)
//
// [----------] Global test environment tear-down
// [==========] 5 tests from 1 test suite ran. (508 ms total)
// [  PASSED  ] 2 tests.
// [  FAILED  ] 3 tests, listed below:
// [  FAILED  ] emlabcpp::testing.simple test
// [  FAILED  ] emlabcpp::testing.simple struct test
// [  FAILED  ] emlabcpp::testing.complex lambda test
//
//  3 FAILED TESTS
