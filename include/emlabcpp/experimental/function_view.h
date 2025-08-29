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

#include <concepts>

namespace emlabcpp
{

template < typename MemberFunctionPtr >
struct _member_function_traits;

template < typename ReturnType, typename Object, typename... ArgTypes >
struct _member_function_traits< ReturnType ( Object::* )( ArgTypes... ) >
{
        using signature = ReturnType( ArgTypes... );
        using object    = Object;
};

template < typename Signature >
class function_view;

template < auto MemberFunctionPtr >
struct member_function
{
        using traits    = _member_function_traits< decltype( MemberFunctionPtr ) >;
        using signature = typename traits::signature;
        using object    = typename traits::object;

        member_function( object& object )
          : obj( object )
        {
        }

        object& obj;
};

template < typename ReturnType, typename... ArgTypes >
class function_view< ReturnType( ArgTypes... ) >
{
public:
        using signature = ReturnType( ArgTypes... );

        function_view( function_view& )                      = default;
        function_view( function_view const& )                = default;
        function_view( function_view&& ) noexcept            = default;
        function_view& operator=( function_view const& )     = default;
        function_view& operator=( function_view&& ) noexcept = default;

        function_view( signature* fun )
          : obj_( reinterpret_cast< void* >( fun ) )
          , handler_( &function_handler )
        {
        }

        template < std::invocable< ArgTypes... > Callable >
        function_view( Callable& cb )
          : obj_( &cb )
          , handler_( &callable_handler< Callable > )
        {
        }

        template < auto MemberFunctionPtr >
        function_view( member_function< MemberFunctionPtr > handle )
          : obj_( &handle.obj )
          , handler_( &member_function_handler<
                      MemberFunctionPtr,
                      typename member_function< MemberFunctionPtr >::object > )
        {
        }

        ReturnType operator()( ArgTypes... args ) const
        {
                return handler_( obj_, (ArgTypes&&) ( args )... );
        }

private:
        template < typename Callable >
        static ReturnType callable_handler( void* const ptr, ArgTypes... args )
        {
                auto* cb_ptr = reinterpret_cast< Callable* >( ptr );
                return ( *cb_ptr )( (ArgTypes&&) ( args )... );
        }

        static ReturnType function_handler( void* const ptr, ArgTypes... args )
        {
                auto f_ptr = reinterpret_cast< signature* >( ptr );
                return f_ptr( args... );
        }

        template < auto MemberFunction, typename Object >
        static ReturnType member_function_handler( void* const ptr, ArgTypes... args )
        {
                auto* obj_ptr = reinterpret_cast< Object* >( ptr );
                return ( obj_ptr->*MemberFunction )( (ArgTypes&&) ( args )... );
        }

        using handler = ReturnType( void*, ArgTypes... );

        void*    obj_;
        handler* handler_;
};

template < auto MemberFunctionPtr >
function_view( member_function< MemberFunctionPtr > handle )
    -> function_view< typename member_function< MemberFunctionPtr >::signature >;

}  // namespace emlabcpp
