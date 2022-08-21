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
#include "emlabcpp/experimental/testing/controller.h"

#ifdef EMLABCPP_USE_GTEST

#pragma once

#include <gtest/gtest.h>

namespace emlabcpp::testing
{

template < ostreamlike T >
T& recursive_print_node(
    T&                  os,
    const testing_tree& t,
    node_id     nid,
    std::size_t         depth )
{

        std::string spacing( depth, ' ' );

        const testing_node* node_ptr = t.get_node( nid );

        if ( node_ptr == nullptr ) {
                return os;
        }

        match(
            node_ptr->get_container_handle(),
            [&]( const value_type& v ) {
                    match(
                        v,
                        [&]( string_buffer val ) {
                                os << std::string_view{ val.begin(), val.size() };
                        },
                        [&]( auto val ) {
                                os << val;
                        } );
            },
            [&]( testing_const_object_handle oh ) {
                    for ( const auto& [key, chid] : oh ) {
                            os << "\n"
                               << spacing << std::string_view{ key.begin(), key.size() } << ":";
                            recursive_print_node( os, t, chid, depth + 1 );
                    }
            },
            [&]( testing_const_array_handle ah ) {
                    for ( const auto& [j, chid] : ah ) {
                            std::ignore = j;
                            os << "\n" << spacing << " - ";
                            recursive_print_node( os, t, chid, depth + 1 );
                    }
            }

        );

        return os;
}

inline ::testing::AssertionResult gtest_predicate( const char*, const testing_result& tres )
{

        ::testing::AssertionResult res = ::testing::AssertionSuccess();
        if ( tres.failed ) {
                res = ::testing::AssertionFailure() << "Test produced a failure, stopping";
        } else if ( tres.errored ) {
                res = ::testing::AssertionFailure() << "Test errored";
        }

        recursive_print_node( res, tres.collected, 0, 1 );
        return res;
}

class testing_gtest : public ::testing::Test
{
        std::optional< controller > opt_con_;
        test_id                     tid_;
        controller_interface&       ci_;

        using mem_type = pool_resource< 256, 32 >;
        mem_type pool_mem_;

public:
        testing_gtest( controller_interface& ci, test_id tid )
          : opt_con_()
          , tid_( tid )
          , ci_( ci )
        {
        }

        void SetUp() final
        {
                opt_con_ = controller::make( ci_, &pool_mem_ );
                if ( !opt_con_ ) {
                        EMLABCPP_LOG( "Failed to build testing controller for gtest" );
                }
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

void testing_register_gtests( controller_interface& ci )
{
        using mem_type = pool_resource< 256, 64 >;
        mem_type pool_mem_;
        auto     opt_con = controller::make( ci, &pool_mem_ );

        if ( !opt_con ) {
                EMLABCPP_LOG( "Failed to build testing controller for gtest registration" );
                return;
        }

        std::string suite_name = std::string{ opt_con->suite_name() };
        for ( auto [tid, tinfo] : opt_con->get_tests() ) {
                std::string     name{ tinfo.name.begin(), tinfo.name.end() };
                test_id test_id = tid;
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

}  // namespace emlabcpp::testing

#endif
