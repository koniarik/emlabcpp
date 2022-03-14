#include "emlabcpp/protocol/streams.h"
#include "emlabcpp/ubjson.h"

#include <iostream>
//
#include "emlabcpp/experimental/testing/controller.h"
#include "emlabcpp/experimental/testing/reactor.h"

#include <deque>
#include <gtest/gtest.h>
#include <thread>

namespace em = emlabcpp;

// ----------------------------------------------------------------------------
// tests definitions

void simple_test_case( em::testing_record& rec )
{
        rec.fail();
}

struct my_test_fixture : em::testing_interface
{
        my_test_fixture() = default;
        // disabling copy should be allowed
        my_test_fixture( const my_test_fixture& ) = delete;
        my_test_fixture& operator=( const my_test_fixture& ) = delete;

        my_test_fixture( my_test_fixture&& ) = default;
        my_test_fixture& operator=( my_test_fixture&& ) = default;

        void setup( em::testing_record& )
        {
        }

        void teardown( em::testing_record& )
        {
        }
};

struct my_test_case : my_test_fixture
{
        void run( em::testing_record& rec )
        {
                rec.collect( "some key for collector", 42 );

                rec.fail();
        }
};

// ----------------------------------------------------------------------------
// virtual communication line via buffers between threads

struct msg_buffer
{
        std::deque< std::vector< uint8_t > > buff_;
        std::mutex                           lock_;

public:
        void insert( std::span< uint8_t > inpt )
        {
                std::lock_guard g{ lock_ };
                buff_.emplace_back( inpt.begin(), inpt.end() );
        }

        bool empty()
        {
                std::lock_guard g{ lock_ };
                return buff_.empty();
        }

        em::static_vector< uint8_t, 64 > pop()
        {
                std::lock_guard g{ lock_ };
                if ( buff_.empty() ) {
                        return {};
                }
                em::static_vector< uint8_t, 64 > res;
                std::copy( buff_.front().begin(), buff_.front().end(), std::back_inserter( res ) );
                buff_.pop_front();
                return res;
        }
};

struct comm : em::testing_reactor_comm_interface
{
        msg_buffer& con_reac_buff;
        msg_buffer& reac_con_buff;

        comm( msg_buffer& cr, msg_buffer& rc )
          : con_reac_buff( cr )
          , reac_con_buff( rc )
        {
        }

        void transmit( std::span< uint8_t > inpt ) final
        {
                reac_con_buff.insert( inpt );
        }

        em::static_vector< uint8_t, 64 > read( std::size_t ) final
        {
                return con_reac_buff.pop();
        }
};

struct con_iface : em::testing_controller_interface
{
        msg_buffer& con_reac_buff;
        msg_buffer& reac_con_buff;

        con_iface( msg_buffer& cr, msg_buffer& rc )
          : con_reac_buff( cr )
          , reac_con_buff( rc )
        {
        }

        void transmit( std::span< uint8_t > inpt ) final
        {
                con_reac_buff.insert( inpt );
        }

        em::static_vector< uint8_t, 64 > read( std::size_t ) final
        {
                return reac_con_buff.pop();
        }

        static ::testing::AssertionResult pred( const char*, const em::testing_result& tres )
        {
                ::testing::AssertionResult res = ::testing::AssertionSuccess();
                if ( tres.failed ) {
                        res = ::testing::AssertionFailure() << "Tests produce a failure, stopping";
                } else if ( tres.errored ) {
                        res = ::testing::AssertionFailure() << "Test errored";
                }
                if ( tres.failed || tres.errored ) {
                        for ( auto [key, arg] : tres.collected_data ) {
                                res << "\n";
                                match( key, [&]( auto val ) {
                                        res << val;
                                } );
                                res << ":";
                                match( arg, [&]( auto val ) {
                                        res << val;
                                } );
                        }
                }
                return res;
        }

        void on_result( em::testing_result res ) final
        {
                EXPECT_PRED_FORMAT1( pred, res );
        }
};

// ----------------------------------------------------------------------------
// register tests and exec virtual setup

class gtest_testing : public ::testing::Test
{
        std::optional< em::testing_controller > opt_con_;
        em::testing_test_id                     tid_;
        em::testing_controller_interface&       ci_;

public:
        gtest_testing( em::testing_controller_interface& ci, em::testing_test_id tid )
          : opt_con_()
          , tid_( tid )
          , ci_( ci )
        {
        }

        void SetUp() final
        {
                opt_con_ = em::testing_controller::make( ci_ );
                EMLABCPP_ASSERT( opt_con_ );
        }

        void TestBody() final
        {
                opt_con_->start_test( tid_, ci_ );
                while ( opt_con_->has_active_test() ) {
                        opt_con_->tick( ci_ );
                }
        }

        void TearDown() final
        {
                opt_con_.reset();
        }
};

int main( int argc, char** argv )
{
        testing::InitGoogleTest( &argc, argv );
        em::testing_default_reactor rec{ "testing test suite" };

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

        msg_buffer con_reac_buff;
        msg_buffer reac_con_buff;

        std::atomic< bool > finished_ = false;

        std::thread t1{ [&] {
                while ( !finished_ ) {
                        if ( !con_reac_buff.empty() ) {
                                auto msg = con_reac_buff.pop();
                                comm c{ con_reac_buff, reac_con_buff };
                                rec.spin( em::view{ msg }, c );
                        }
                        std::chrono::milliseconds t{ 10 };
                        std::this_thread::sleep_for( t );
                }
        } };

        con_iface ci{ con_reac_buff, reac_con_buff };
        auto      opt_con = em::testing_controller::make( ci );
        for ( auto [tid, tinfo] : opt_con->get_tests() ) {
                std::string name{ tinfo.name.begin(), tinfo.name.end() };
                ::testing::RegisterTest(
                    "emlabcpp_testing",
                    name.c_str(),
                    nullptr,
                    nullptr,
                    __FILE__,
                    __LINE__,
                    [&ci, tid] {
                            return new gtest_testing( ci, tid );
                    } );
        }

        auto res = RUN_ALL_TESTS();

        finished_ = true;
        t1.join();

        em::ignore(res);
        // TODO: temporary solution 
        return 0;
}

