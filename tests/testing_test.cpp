#include "emlabcpp/experimental/testing/controller.h"
#include "emlabcpp/experimental/testing/gtest.h"
#include "emlabcpp/experimental/testing/reactor.h"
#include "emlabcpp/protocol/streams.h"
#include "util.h"

#include <gtest/gtest.h>

namespace em = emlabcpp;

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

void simple_test_case( em::testing_record& rec )
{
        // showcase for the record API

        // request an argument from controller
        em::testing_arg_variant var        = rec.get_arg( "arg1" );
        uint64_t                config_val = std::get< uint64_t >( var );

        for ( uint32_t i = 0; i < config_val; i++ ) {
                auto var2   = rec.get_arg( i );
                std::ignore = var2;
        }

        // collecting records data in the controller
        // this is stored for later review
        rec.collect( "col1", "some_data" );
        rec.collect( "col2", 42 );
        rec.collect( 33, "foooo" );

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

struct my_test_fixture : em::testing_interface
{
        // The basic test unit is actually "em::testing_interface"
        // and it's run() method.
        //
        // `simple_test_case` is just called in run() method of
        // lambda handler for the testing_interface
        //
        // Apart from run, the test can also implement
        // setup/teardown methods for preparation of the tests.
        // If this methods fail (via rec.fail()) that is taken
        // as critical failure and run() is not executed.
        //
        // This can lead to preparing one test "fixture"
        // that executes preparation common in other tests
        // which just implement the run() method.

        my_test_fixture() = default;
        // disabling copy should be allowed
        my_test_fixture( const my_test_fixture& ) = delete;
        my_test_fixture& operator=( const my_test_fixture& ) = delete;

        my_test_fixture( my_test_fixture&& ) = default;
        my_test_fixture& operator=( my_test_fixture&& ) = default;

        void setup( em::testing_record& ) override
        {
                // setup i2c
        }

        void teardown( em::testing_record& ) override
        {
                // teardown i2c
        }
};

struct my_test_case : my_test_fixture
{
        // testing class using basic fixture for preparation
        // tadaaaaah!

        void run( em::testing_record& rec ) override
        {
                rec.collect( "some key for collector", 42 );

                rec.fail();
        }
};

// ----------------------------------------------------------------------------
// interfaces that has to be implemented for the framework objects

struct reactor_iface : em::testing_reactor_interface
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

        // Argument specifies ideal number of bytes that should be read
        // More is not allowed, less is aallowed
        std::optional< em::static_vector< uint8_t, 64 > > receive( std::size_t ) final
        {
                return con_reac_buff.pop();
        }
};

// TODO: an idea
//  - split the controll interface into controller/controller_com interface
//  - most of the API for controller can be implemented specifically for gtest
//  - only thing that remains is the "comm" api
//  - if separated, that abstraction is easily possible
struct controller_iface : em::testing_controller_interface
{
        em::thread_safe_queue& con_reac_buff;
        em::thread_safe_queue& reac_con_buff;

        controller_iface( em::thread_safe_queue& cr, em::thread_safe_queue& rc )
          : con_reac_buff( cr )
          , reac_con_buff( rc )
        {
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
        void on_result( em::testing_result res ) final
        {
                EXPECT_PRED_FORMAT1( em::testing_gtest_predicate, res );
        }

        // if the test requires an argument, the request
        // is propagate to the interface and should be solved
        // on this level
        std::optional< em::testing_arg_variant > on_arg( std::string_view key ) final
        {
                if ( key == "arg1" ) {
                        return 2lu;
                }

                if ( key == "arg_key" ) {
                        return false;
                }

                return {};
        }

        std::optional< em::testing_arg_variant > on_arg( uint32_t key ) final
        {
                if ( key < 2u ) {
                        return false;
                }

                return {};
        }

        void on_error( em::testing_error_variant err ) final
        {
                FAIL() << err;
        }
};

int main( int argc, char** argv )
{
        testing::InitGoogleTest( &argc, argv );

        // ----------------------------------------------------------------------------
        // register tests and examples of lambda tests

        em::testing_default_reactor rec{ "emlabcpp::testing" };

        rec.register_callable( "simple test", simple_test_case );
        rec.register_callable( "simple lambda test", [&]( em::testing_record& rec ) {
                rec.success();
        } );
        rec.register_test( "simple struct test", my_test_case{} );

        rec.register_callable( "complex lambda test", [&]( em::testing_record& rec ) {
                em::testing_arg_variant data = rec.get_arg( "arg_key" );

                if ( !std::holds_alternative< uint64_t >( data ) ) {
                        rec.success();
                } else {
                        rec.fail();
                }
        } );

        rec.register_test(
            "lambda amd fixture",
            em::testing_compose< my_test_fixture >( [&]( em::testing_record& rec ) {
                    rec.expect( 1 > 0 );
            } ) );

        // ----------------------------------------------------------------------------
        // build the virtual example and run it

        em::thread_safe_queue con_reac_buff;
        em::thread_safe_queue reac_con_buff;

        std::atomic< bool > finished_ = false;

        std::thread t1{ [&] {
                reactor_iface             iface{ con_reac_buff, reac_con_buff };
                std::chrono::milliseconds t{ 10 };

                while ( !finished_ ) {
                        rec.spin( iface );
                        std::this_thread::sleep_for( t );
                }
        } };

        controller_iface ci{ con_reac_buff, reac_con_buff };

        em::testing_register_gtests( ci );

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
