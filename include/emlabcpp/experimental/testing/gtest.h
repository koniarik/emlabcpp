#include "emlabcpp/experimental/testing/controller.h"

#ifdef EMLABCPP_USE_GTEST

#pragma once

#include <gtest/gtest.h>

namespace emlabcpp
{

inline ::testing::AssertionResult testing_gtest_predicate( const char*, testing_result& tres )
{

        ::testing::AssertionResult res = ::testing::AssertionSuccess();
        if ( tres.failed ) {
                res = ::testing::AssertionFailure() << "Test produced a failure, stopping";
        } else if ( tres.errored ) {
                res = ::testing::AssertionFailure() << "Test errored";
        }

        if ( !tres.collected_data.empty() ) {
                res << "\ncollected:";
        }

        for ( auto [key, arg] : tres.collected_data ) {
                res << "\n ";
                match(
                    key,
                    [&]( uint32_t val ) {
                            res << val;
                    },
                    [&]( testing_key_buffer val ) {
                            res << std::string_view{ val.begin(), val.size() };
                    } );
                res << " :\t";
                match(
                    arg,
                    [&]( testing_string_buffer val ) {
                            res << std::string_view{ val.begin(), val.size() };
                    },
                    [&]( auto val ) {
                            res << val;
                    } );
        }

        return res;
}

class testing_gtest : public ::testing::Test
{
        std::optional< testing_controller > opt_con_;
        testing_test_id                     tid_;
        testing_controller_interface&       ci_;

        using mem_type = pool_resource< 88, 32 >;
        mem_type pool_mem_;

public:
        testing_gtest( testing_controller_interface& ci, testing_test_id tid )
          : opt_con_()
          , tid_( tid )
          , ci_( ci )
        {
        }

        void SetUp() final
        {
                opt_con_ = testing_controller::make( ci_, &pool_mem_ );
                ASSERT_TRUE( opt_con_ );
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

void testing_register_gtests( testing_controller_interface& ci )
{
        using mem_type = pool_resource< 88, 32 >;
        mem_type pool_mem_;
        auto     opt_con = testing_controller::make( ci, &pool_mem_ );

        if ( !opt_con ) {
                std::cout << "Failed to initialize the controller" << std::endl;
                return;
        }

        std::string suite_name = std::string{ opt_con->suite_name() };
        for ( auto [tid, tinfo] : opt_con->get_tests() ) {
                std::string     name{ tinfo.name.begin(), tinfo.name.end() };
                testing_test_id test_id = tid;
                ::testing::RegisterTest(
                    suite_name.c_str(),
                    name.c_str(),
                    nullptr,
                    nullptr,
                    __FILE__,
                    __LINE__,
                    [&ci, test_id] {
                            return new testing_gtest( ci, test_id );
                    } );
        }
}

}  // namespace emlabcpp

#endif
