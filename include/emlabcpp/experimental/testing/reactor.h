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
#include "emlabcpp/allocator/pool.h"
#include "emlabcpp/assert.h"
#include "emlabcpp/defer.h"
#include "emlabcpp/experimental/testing/interface.h"
#include "emlabcpp/experimental/testing/protocol.h"
#include "emlabcpp/experimental/testing/reactor_interface.h"
#include "emlabcpp/protocol/packet_handler.h"
#include "emlabcpp/view.h"
#include "emlabcpp/visit.h"

#include <list>

#pragma once

namespace emlabcpp
{

class testing_reactor
{

        struct test_handle
        {
                std::string_view   name;
                testing_interface* ptr;

                test_handle( std::string_view n, testing_interface* p )
                  : name( n )
                  , ptr( p )
                {
                }
        };

        struct active_execution
        {
                testing_test_id tid;
                testing_run_id  rid;
                test_handle*    handle_ptr;
        };

        using handle_container = pool_list< test_handle >;
        using handle_iterator  = typename handle_container::iterator;
        using sequencer        = testing_controller_reactor_packet::sequencer;

        std::string_view suite_name_;
        std::string_view suite_date_ = __DATE__;
        handle_container handles_;
        pool_interface*  mem_;
        sequencer        seq_;

        std::optional< active_execution > active_exec_;

public:
        testing_reactor( std::string_view suite_name, pool_interface* mem )
          : suite_name_( suite_name )
          , handles_( mem )
          , mem_( mem )
        {
        }

        testing_reactor( const testing_reactor& )            = delete;
        testing_reactor( testing_reactor&& )                 = delete;
        testing_reactor& operator=( const testing_reactor& ) = delete;
        testing_reactor& operator=( testing_reactor&& )      = delete;

        template < testing_test T >
        bool register_test( std::string_view name, T t )
        {
                return store_test( name, std::move( t ) );
        }

        template < testing_callable T >
        bool register_callable( std::string_view name, T cb )
        {
                return store_test( name, testing_callable_overlay{ std::move( cb ) } );
        }

        void spin( testing_reactor_interface& comm );

private:
        void handle_message( tag< TESTING_SUITE_NAME >, testing_reactor_interface_adapter& );
        void handle_message( tag< TESTING_SUITE_DATE >, testing_reactor_interface_adapter& );
        void handle_message( tag< TESTING_COUNT >, testing_reactor_interface_adapter& );
        void handle_message(
            tag< TESTING_NAME >,
            testing_test_id tid,
            testing_reactor_interface_adapter& );
        void handle_message(
            tag< TESTING_LOAD >,
            testing_test_id tid,
            testing_run_id  rid,
            testing_reactor_interface_adapter& );
        void
        handle_message( tag< TESTING_EXEC >, testing_run_id, testing_reactor_interface_adapter& );

        template < testing_test T >
        bool store_test( std::string_view name, T t )
        {
                void* target = mem_->allocate( sizeof( T ), alignof( T ) );
                if ( target == nullptr ) {
                        EMLABCPP_LOG( "Failed to allocate memory for storing test " << name );
                        return false;
                }
                T* obj = std::construct_at( reinterpret_cast< T* >( target ), std::move( t ) );
                handles_.emplace_back( name, obj );
                return true;
        }

        static void test_deleter( pool_interface& mem, void* p )
        {
                mem.deallocate( p );
        }

        void exec_test( testing_reactor_interface_adapter& comm );

        test_handle& access_test( testing_test_id );

public:
        ~testing_reactor();
};

class testing_default_reactor : private pool_base< 128, 32 >, public testing_reactor
{
public:
        /// TODO: this may not be the best idea, as pool_mem will exist only _after_ constructor
        /// for base class is called
        testing_default_reactor( std::string_view name )
          : testing_reactor( name, &this->pool_memory )
        {
        }
};

}  // namespace emlabcpp
