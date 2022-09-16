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
        template < with_signature< ReturnType( ArgTypes... ) > Callable >
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
        static ReturnType CallableHandler( void* ptr, ArgTypes... args )
        {
                Callable* cb_ptr = reinterpret_cast< Callable* >( ptr );
                return ( *cb_ptr )( std::forward< ArgTypes >( args )... );
        }

        using handler = ReturnType( void*, ArgTypes... );

        void*    obj_;
        handler* handler_;
};

}  // namespace emlabcpp
