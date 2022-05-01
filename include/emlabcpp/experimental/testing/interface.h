#include "emlabcpp/experimental/testing/record.h"
#include "emlabcpp/protocol/packet_handler.h"

#include <span>

#pragma once

namespace emlabcpp
{

class testing_interface
{
public:
        virtual void setup( testing_record& )
        {
        }
        virtual void run( testing_record& ) = 0;
        virtual void teardown( testing_record& )
        {
        }

        virtual ~testing_interface() = default;
};

template < typename T >
concept testing_test = std::derived_from< T, testing_interface >;  /// && std::movable< T >;

template < typename T >
concept testing_callable = requires( T t, testing_record& rec )
{
        t( rec );
};

template < testing_callable Callable >
class testing_callable_overlay : public testing_interface
{
        Callable cb_;

public:
        testing_callable_overlay( Callable cb )
          : cb_( std::move( cb ) )
        {
        }

        void run( testing_record& rec ) final
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

        void run( testing_record& rec ) final
        {
                cb_( rec );
        }
};

template < testing_test T, testing_callable C >
testing_composer< T, C > testing_compose( T t, C c )
{
        return { std::move( t ), std::move( c ) };
}

}  // namespace emlabcpp
