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

namespace emlabcpp::testing
{

class reactor
{

        struct test_handle
        {
                std::string_view name;
                test_interface*  ptr;
                std::size_t      alignment;

                test_handle( std::string_view n, test_interface* p, std::size_t a )
                  : name( n )
                  , ptr( p )
                  , alignment( a )
                {
                }
        };

        struct active_execution
        {
                test_id      tid;
                run_id       rid;
                test_handle* handle_ptr;
        };

        using handle_container = pool_list< test_handle >;
        using handle_iterator  = typename handle_container::iterator;

        std::string_view suite_name_;
        std::string_view suite_date_ = __DATE__ " " __TIME__;
        handle_container handles_;
        pool_interface*  mem_;
        reactor_endpoint ep_;

        std::optional< active_execution > active_exec_;

public:
        reactor( std::string_view suite_name, pool_interface* mem )
          : suite_name_( suite_name )
          , handles_( mem )
          , mem_( mem )
        {
        }

        reactor( const reactor& )            = delete;
        reactor( reactor&& )                 = delete;
        reactor& operator=( const reactor& ) = delete;
        reactor& operator=( reactor&& )      = delete;

        template < std::derived_from< test_interface > T >
        bool register_test( std::string_view name, T t )
        {
                return store_test( name, std::move( t ) );
        }

        template < test_callable T >
        bool register_callable( std::string_view name, T cb )
        {
                return store_test( name, test_callable_overlay{ std::move( cb ) } );
        }

        void spin( reactor_interface& comm );

private:
        void handle_message( get_property< SUITE_NAME >, reactor_interface_adapter& );
        void handle_message( get_property< SUITE_DATE >, reactor_interface_adapter& );
        void handle_message( get_property< COUNT >, reactor_interface_adapter& );
        void handle_message( get_test_name_request, reactor_interface_adapter& );
        void handle_message( load_test, reactor_interface_adapter& );
        void handle_message( exec_request, reactor_interface_adapter& );

        template < std::derived_from< test_interface > T >
        bool store_test( std::string_view name, T t )
        {
                void* target = mem_->allocate( sizeof( T ), alignof( T ) );
                if ( target == nullptr ) {
                        EMLABCPP_LOG( "Failed to allocate memory for storing test " << name );
                        return false;
                }
                T* obj = std::construct_at( reinterpret_cast< T* >( target ), std::move( t ) );
                handles_.emplace_back( name, obj, alignof( T ) );
                return true;
        }

        void exec_test( reactor_interface_adapter& comm );

        test_handle& access_test( test_id );

public:
        ~reactor();
};

class default_reactor : private pool_base< 512, 32 >, public reactor
{
public:
        /// TODO: this may not be the best idea, as pool_mem will exist only _after_ constructor
        /// for base class is called
        default_reactor( std::string_view name )
          : reactor( name, &this->pool_memory )
        {
        }
};

}  // namespace emlabcpp::testing
