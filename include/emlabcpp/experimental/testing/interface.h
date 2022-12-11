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
#include "emlabcpp/experimental/binding_linked_list.h"
#include "emlabcpp/experimental/testing/coroutine.h"
#include "emlabcpp/experimental/testing/record.h"
#include "emlabcpp/protocol/packet_handler.h"

#include <span>

#pragma once

namespace emlabcpp::testing
{

class reactor;
class empty_test;

class test_interface : public binding_linked_list_node< test_interface >
{
public:
        test_interface( reactor& rec, const name_buffer& name );
        test_interface( reactor& rec, std::string_view name );

        test_interface( const test_interface& )            = delete;
        test_interface& operator=( const test_interface& ) = delete;

        test_interface( test_interface&& other ) = default;

        test_interface& operator=( test_interface&& ) = delete;

        virtual test_coroutine setup( pmr::memory_resource&, record& );
        virtual test_coroutine run( pmr::memory_resource&, record& ) = 0;
        virtual test_coroutine teardown( pmr::memory_resource&, record& );

        virtual ~test_interface() = default;

        friend reactor;
        friend empty_test;

private:
        test_interface() = default;

        using binding_linked_list_node< test_interface >::push_after;

        name_buffer name;
};

class empty_test : public test_interface
{
public:
        explicit empty_test( reactor& rec )
          : test_interface( rec, name_buffer{} )
        {
        }

        test_coroutine run( pmr::memory_resource&, record& ) override
        {
                co_return;
        };

        friend reactor;

private:
        empty_test() = default;
};

template < typename T >
concept valid_test_callable = requires( T t, pmr::memory_resource& mem_resource, record& rec )
{
        {
                t( mem_resource, rec )
                } -> std::same_as< test_coroutine >;
};

template < valid_test_callable Callable >
class test_callable : public test_interface
{
        Callable cb_;

public:
        test_callable( reactor& rec, const std::string_view name, Callable cb )
          : test_interface( rec, name_to_buffer( name ) )
          , cb_( std::move( cb ) )
        {
        }

        test_coroutine run( pmr::memory_resource& mem_resource, record& rec ) final
        {
                return cb_( mem_resource, rec );
        }
};

template < std::derived_from< test_interface > T, valid_test_callable C >
class test_composer : public T
{
        C cb_;

public:
        test_composer( T t, C c )
          : T( std::move( t ) )
          , cb_( std::move( c ) )
        {
        }

        test_coroutine run( pmr::memory_resource& mem_resource, record& rec ) final
        {
                return cb_( mem_resource, rec );
        }
};

template < std::derived_from< test_interface > T, valid_test_callable C >
test_composer< T, C > test_compose( T t, C c )
{
        return { std::move( t ), std::move( c ) };
}

}  // namespace emlabcpp::testing
