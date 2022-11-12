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
        virtual test_coroutine setup( pool_interface*, record& )
        {
                co_return;
        }
        virtual test_coroutine run( pool_interface*, record& ) = 0;
        virtual test_coroutine teardown( pool_interface*, record& )
        {
                co_return;
        }

        virtual ~test_interface() = default;
};

template < typename T >
concept test_callable = requires( T t, pool_interface* pool, record& rec )
{
        {
                t( pool, rec )
                } -> std::same_as< test_coroutine >;
};

template < test_callable Callable >
class test_callable_overlay : public test_interface
{
        Callable cb_;

public:
        test_callable_overlay( Callable cb )
          : cb_( std::move( cb ) )
        {
        }

        test_coroutine run( pool_interface* pool, record& rec ) final
        {
                return cb_( pool, rec );
        }
};

template < std::derived_from< test_interface > T, test_callable C >
class test_composer : public T
{
        C cb_;

public:
        test_composer( T t, C c )
          : T( std::move( t ) )
          , cb_( std::move( c ) )
        {
        }

        test_coroutine run( pool_interface* pool, record& rec ) final
        {
                return cb_( pool, rec );
        }
};

template < std::derived_from< test_interface > T, test_callable C >
test_composer< T, C > test_compose( T t, C c )
{
        return { std::move( t ), std::move( c ) };
}

}  // namespace emlabcpp::testing
