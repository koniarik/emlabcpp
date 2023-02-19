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
#include "emlabcpp/experimental/linked_list.h"
#include "emlabcpp/experimental/testing/coroutine.h"
#include "emlabcpp/experimental/testing/record.h"
#include "emlabcpp/protocol/packet_handler.h"

#include <span>

#pragma once

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

        virtual test_coroutine setup( pmr::memory_resource&, record& );
        virtual test_coroutine run( pmr::memory_resource&, record& ) = 0;
        virtual test_coroutine teardown( pmr::memory_resource&, record& );

        virtual ~test_interface() = default;
};

using test_ll_node = linked_list_node< test_interface* >;

template < typename T >
concept valid_test_callable = requires( T t, pmr::memory_resource& mem_resource, record& rec )
{
        {
                t( mem_resource, rec )
                } -> std::same_as< test_coroutine >;
};

template < std::derived_from< test_interface > T >
class test_unit
{
public:
        template < typename... Args >
        test_unit( auto& reac, Args&&... args )
          : item_( std::forward< Args >( args )... )
          , node_( &item_ )
        {
                reac.register_test( node_ );
        }

        test_unit( const test_unit& )            = delete;
        test_unit( test_unit&& )                 = delete;
        test_unit& operator=( const test_unit& ) = delete;
        test_unit& operator=( test_unit&& )      = delete;

        T& operator*()
        {
                return item_;
        }
        const T& operator*() const
        {
                return item_;
        }

        T* operator->()
        {
                return &item_;
        }
        const T* operator->() const
        {
                return &item_;
        }

private:
        T            item_;
        test_ll_node node_;
};

template < valid_test_callable Callable >
class test_callable : public test_interface
{
public:
        test_callable( auto& reac, const std::string_view name, Callable cb )
          : node_( this )
          , name_( name_to_buffer( name ) )
          , cb_( std::move( cb ) )
        {
                reac.register_test( node_ );
        }

        test_callable( test_callable&& ) noexcept            = default;
        test_callable& operator=( test_callable&& ) noexcept = default;

        [[nodiscard]] std::string_view get_name() const override
        {
                return std::string_view{ name_.data(), name_.size() };
        }

        test_coroutine run( pmr::memory_resource& mem_resource, record& rec ) final
        {
                return cb_( mem_resource, rec );
        }

private:
        test_ll_node node_;
        name_buffer  name_;
        Callable     cb_;
};

}  // namespace emlabcpp::testing
