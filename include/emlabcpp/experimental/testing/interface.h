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
#include "emlabcpp/experimental/testing/record.h"
#include "emlabcpp/protocol/packet_handler.h"

#include <span>

#pragma once

namespace emlabcpp::testing
{

class test_interface
{
public:
        virtual void setup( record& )
        {
        }
        virtual void run( record& ) = 0;
        virtual void teardown( record& )
        {
        }

        virtual ~test_interface() = default;
};

template < typename T >
concept testing_test = std::derived_from< T, test_interface >;  /// && std::movable< T >;

template < typename T >
concept testing_callable = requires( T t, record& rec )
{
        t( rec );
};

template < testing_callable Callable >
class testing_callable_overlay : public test_interface
{
        Callable cb_;

public:
        testing_callable_overlay( Callable cb )
          : cb_( std::move( cb ) )
        {
        }

        void run( record& rec ) final
        {
                cb_( rec );
        }
};

template < testing_test T, testing_callable C >
class testing_composer : public T
{
        C cb_;

public:
        testing_composer( T t, C c )
          : T( std::move( t ) )
          , cb_( std::move( c ) )
        {
        }

        void run( record& rec ) final
        {
                cb_( rec );
        }
};

template < testing_test T, testing_callable C >
testing_composer< T, C > testing_compose( T t, C c )
{
        return { std::move( t ), std::move( c ) };
}

}  // namespace emlabcpp::testing
