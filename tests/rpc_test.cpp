#include "emlabcpp/experimental/rpc.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

enum foo_ids : uint8_t
{
        CALL_1
};

struct foo
{
        int call1_m( int i )
        {
                return i * 2;
        }
};

using call1 = rpc::derive< CALL_1, &foo::call1_m >;

using test_wrapper = rpc::class_wrapper< foo, call1 >;

TEST( rpc, basic )
{
        foo f;

        test_wrapper wrap{ f };

        using con = rpc::controller< test_wrapper::call_defs >;

        con::call< CALL_1 >(
            call1::request{ 42 },
            [&]( const auto& msg ) {
                    return wrap.on_message( msg );
            } )
            .match(
                [&]( int r ) {
                        EXPECT_EQ( r, 84 );
                },
                [&]( auto ) {
                        FAIL() << "got an error";
                } );
}
