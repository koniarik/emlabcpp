///
/// Copyright (C) 2020 Jan Veverak Koniarik
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy of this software
/// and associated documentation files (the "Software"), to deal in the Software without
/// restriction, including without limitation the rights to use, copy, modify, merge, publish,
/// distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
/// Software is furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in all copies or
/// substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
/// BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
/// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
///

#include "emlabcpp/concepts.h"

#pragma once

namespace emlabcpp
{

template < typename Signature >
class function_view;

template < typename ReturnType, typename... ArgTypes >
class function_view< ReturnType( ArgTypes... ) >
{
public:
        using signature = ReturnType( ArgTypes... );

        function_view( function_view& )                      = default;
        function_view( const function_view& )                = default;
        function_view( function_view&& ) noexcept            = default;
        function_view& operator=( const function_view& )     = default;
        function_view& operator=( function_view&& ) noexcept = default;

        function_view( signature* fun )
          : obj_( reinterpret_cast< void* >( fun ) )
          , handler_( &FunctionHandler )
        {
        }

        template < std::invocable< ArgTypes... > Callable >
        function_view( Callable& cb )
          : obj_( &cb )
          , handler_( &CallableHandler< Callable > )
        {
        }

        ReturnType operator()( ArgTypes... args ) const
        {
                return handler_( obj_, std::forward< ArgTypes >( args )... );
        }

private:
        template < typename Callable >
        static ReturnType CallableHandler( void* const ptr, ArgTypes... args )
        {
                auto* cb_ptr = reinterpret_cast< Callable* >( ptr );
                return ( *cb_ptr )( std::forward< ArgTypes >( args )... );
        }

        static ReturnType FunctionHandler( void* const ptr, ArgTypes... args )
        {
                auto f_ptr = reinterpret_cast< signature* >( ptr );
                return f_ptr( args... );
        }

        using handler = ReturnType( void*, ArgTypes... );

        void*    obj_;
        handler* handler_;
};

}  // namespace emlabcpp
