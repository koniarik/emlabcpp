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

namespace emlabcpp
{

template < ostreamlike T >
inline T&
testing_recursive_print_node( T& os, const testing_tree& t, testing_node_id nid, std::size_t depth )
{

        std::string spacing( depth, ' ' );

        const testing_node* node_ptr = t.get_node( nid );

        if ( !node_ptr ) {
                return os;
        }

        match(
            node_ptr->get_container_handle(),
            [&]( const testing_value* v ) {
                    match(
                        *v,
                        [&]( testing_string_buffer val ) {
                                os << std::string_view{ val.begin(), val.size() };
                        },
                        [&]( auto val ) {
                                os << val;
                        } );
            },
            [&]( testing_const_object_handle oh ) {
                    uint32_t ch_count = oh.size();
                    for ( uint32_t i : range< uint32_t >( ch_count ) ) {
                            const testing_key* key = oh.get_key( i );
                            EMLABCPP_ASSERT( key );
                            os << "\n" << spacing << std::string_view{key->begin(), key->size()} << ":";
                            std::optional< testing_node_id > opt_j = oh.get_child( i );
                            EMLABCPP_ASSERT( opt_j );
                            testing_recursive_print_node( os, t, *opt_j, depth + 1 );
                    }
            },
            [&]( testing_const_array_handle ah ) {
                    uint32_t ch_count = ah.size();
                    for ( uint32_t i : range< uint32_t >( ch_count ) ) {
                            os << "\n" << spacing << " - ";
                            std::optional< testing_node_id > opt_j = ah.get_child( i );
                            EMLABCPP_ASSERT( opt_j );
                            testing_recursive_print_node( os, t, *opt_j, depth + 1 );
                    }
            }

        );

        return os;
}

inline ::testing::AssertionResult testing_gtest_predicate( const char*, const testing_result& tres )
{

        ::testing::AssertionResult res = ::testing::AssertionSuccess();
        if ( tres.failed ) {
                res = ::testing::AssertionFailure() << "Test produced a failure, stopping";
        } else if ( tres.errored ) {
                res = ::testing::AssertionFailure() << "Test errored";
        }

        testing_recursive_print_node( res, tres.collected, 0, 1 );
        return res;
}

class testing_gtest : public ::testing::Test
{
        std::optional< testing_controller > opt_con_;
        testing_test_id                     tid_;
        testing_controller_interface&       ci_;

        using mem_type = pool_resource< 256, 32 >;
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
        using mem_type = pool_resource< 256, 64 >;
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
