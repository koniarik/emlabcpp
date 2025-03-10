/// MIT License
///
/// Copyright (c) 2025 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
///

#pragma once

#include "emlabcpp/experimental/linked_list.h"
#include "emlabcpp/experimental/testing/base.h"
#include "emlabcpp/experimental/testing/coroutine.h"
#include "emlabcpp/protocol/packet_handler.h"
#include "emlabcpp/result.h"

#include <span>

namespace emlabcpp::testing
{

class test_interface
{
public:
        test_interface() = default;

        test_interface( const test_interface& )            = delete;
        test_interface& operator=( const test_interface& ) = delete;
        test_interface( test_interface&& other )           = default;
        test_interface& operator=( test_interface&& )      = default;

        [[nodiscard]] virtual std::string_view get_name() const = 0;

        virtual coroutine< void > setup( pmr::memory_resource& );
        virtual coroutine< void > run( pmr::memory_resource& ) = 0;
        virtual coroutine< void > teardown( pmr::memory_resource& );

        virtual ~test_interface() = default;
};

using test_ll_node = linked_list_node_base< test_interface >;

template < typename T >
concept valid_test_callable = requires( T t, pmr::memory_resource& mem_resource ) {
        { t( mem_resource ) } -> std::same_as< coroutine< void > >;
};

template < typename T >
using test_unit = linked_list_node< T, test_interface >;

template < valid_test_callable Callable >
class test_callable : public test_interface
{
public:
        test_callable( const std::string_view name, Callable cb )
          : name_( name )
          , cb_( std::move( cb ) )
        {
        }

        test_callable( const std::string_view name, auto& rec, Callable cb )
          : name_( name )
          , cb_( std::move( cb ) )
        {
                rec.register_test( this );
        }

        test_callable( test_callable&& ) noexcept            = default;
        test_callable& operator=( test_callable&& ) noexcept = default;

        [[nodiscard]] std::string_view get_name() const override
        {
                return std::string_view{ name_.data(), name_.size() };
        }

        coroutine< void > run( pmr::memory_resource& mem_resource ) final
        {
                return cb_( mem_resource );
        }

private:
        name_buffer name_;
        Callable    cb_;
};

template < valid_test_callable Callable >
test_callable( const std::string_view name, Callable cb ) -> test_callable< Callable >;

struct empty_node_impl : test_interface
{
        [[nodiscard]] std::string_view get_name() const override
        {
                return "";
        }

        coroutine< void > run( pmr::memory_resource& ) override
        {
                co_return;
        }
};

using empty_node = test_unit< empty_node_impl >;

template < valid_test_callable Callable >
using test_linked_callable = test_unit< test_callable< Callable > >;

// TODO: the sad thing is that they won't be deallocated this way /o\...
template < typename Unit, typename Reactor, typename... Args >
result construct_test_unit( pmr::memory_resource& mem_res, Reactor& reactor, Args&&... args )
{
        using unode = test_unit< Unit >;
        void* p     = mem_res.allocate( sizeof( unode ), alignof( unode ) );
        if ( p == nullptr )
                return ERROR;
        unode* node =
            std::construct_at( reinterpret_cast< unode* >( p ), std::forward< Args >( args )... );

        reactor.register_test( *node );

        return SUCCESS;
}

template < typename Callable, typename Reactor >
result construct_test_callable(
    pmr::memory_resource& mem_res,
    Reactor&              reactor,
    std::string_view      name,
    Callable              callable )
{
        return construct_test_unit< test_callable< Callable > >(
            mem_res, reactor, name, std::move( callable ) );
}

template < typename Callable, typename Reactor >
void construct_test_callable(
    pmr::memory_resource& mem_res,
    Reactor&              reactor,
    std::string_view      name,
    Callable              callable,
    result&               res )
{
        if ( res != SUCCESS )
                return;
        res = construct_test_unit< test_callable< Callable > >(
            mem_res, reactor, name, std::move( callable ) );
}

}  // namespace emlabcpp::testing
