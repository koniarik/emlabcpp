
#pragma once

namespace emlabcpp
{

class testing_interface
{
        virtual void setup( em::testing_record& )
        {
        }
        virtual void run( em::testing_record& ) = 0;
        virtual void teardown( em::testing_record& )
        {
        }

        virtual ~testing_interface() = default;
};

template < typename Callable >
class testing_callable_overlay : testing_interface
{
        Callable cb_;

public:
        testing_callable_overlay( Callable cb )
          : cb_( std::move( cb ) )
        {
        }

        void run( em::testing_record& rec )
        {
                cb_( rec );
        }
};

}  // namespace emlabcpp
