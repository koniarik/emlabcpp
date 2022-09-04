#include "emlabcpp/experimental/rpc.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

struct call1
{
        struct request
        {
                int i;
        };

        using reply = int;
};

struct foo
{
        int call1_m( int i )
        {
                return i * 2;
        }
};

struct test_wrapper
{
        foo& val_;

        using call_defs            = std::tuple< call1 >;
        using reactor              = rpc::reactor< call_defs >;
        using request_message_type = typename reactor::request_message_type;
        using reply_message_type   = typename reactor::reply_message_type;

        reply_message_type on_message( const request_message_type& msg )
        {
                return reactor::on_message( msg, *this );
        }

        call1::reply operator()( call1::request r )
        {
                return val_.call1_m( r.i );
        }
};

TEST( rpc, basic )
{
        foo f;

        test_wrapper wrap{ f };

        using con = rpc::controller< test_wrapper::call_defs >;

        con::call< call1 >(
            call1::request{ 42 },
            [&]( const auto& msg ) {
                    return wrap.on_message( msg );
            } )
            .match(
                [&]( call1::reply r ) {
                        EXPECT_EQ( r, 84 );
                },
                [&]( auto ) {
                        FAIL() << "got an error";
                } );
}
