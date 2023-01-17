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

        function_view( signature* fun )
          : obj_( reinterpret_cast< void* >( fun ) )
          , handler_( &FunctionHandler )
        {
        }

        template < typename Callable >
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
