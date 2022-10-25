#include "emlabcpp/experimental/rpc.h"

#include "emlabcpp/experimental/pretty_printer.h"

#include <gtest/gtest.h>

using namespace emlabcpp;

enum foo_ids : uint8_t
{
        CALL_1 = 1,
        CALL_2 = 2,
        CALL_3 = 3
};

struct foo
{
        int call1_m( int i )
        {
                return i * 2;
        }

        std::tuple< int, int > call2_m( float val, int32_t val2 )
        {
                return { static_cast< int >( val * static_cast< float >( val2 ) ), 42 };
        }

        void call3_m( int )
        {
        }
};

using direct_test_wrapper = rpc::class_wrapper<
    foo,
    rpc::derive< CALL_1, &foo::call1_m >,
    rpc::derive< CALL_2, &foo::call2_m >,
    rpc::derive< CALL_3, &foo::call3_m > >;

using bind_test_wrapper = rpc::bind_wrapper<
    rpc::bind< CALL_1, int( int ) >,
    rpc::bind< CALL_2, std::tuple< int, int >( float, int32_t ) >,
    rpc::bind< CALL_3, void( int ) > >;

TEST( rpc, basic )
{
        foo f;

        direct_test_wrapper wrap{ f };

        // this is callback used to send and receive message between devices, in tests it's just
        // direct exchange
        auto exchange_messages_f = [&]( const auto& msg ) {
                return wrap.on_message( msg );
        };

        using con = rpc::controller< direct_test_wrapper >;

        con::call< CALL_1 >( exchange_messages_f, 42 )
            .match(
                [&]( int r ) {
                        EXPECT_EQ( r, 84 );
                },
                [&]( auto ) {
                        FAIL() << "got an error";
                } );

        con::call< CALL_2 >( exchange_messages_f, 0.4f, 666u )
            .match(
                [&]( std::tuple< int, int > res ) {
                        auto [a, b] = res;
                        EXPECT_EQ( a, 266 );
                        EXPECT_EQ( b, 42 );
                },
                [&]( auto ) {
                        FAIL() << "got an error";
                } );

        con::call< CALL_3 >( exchange_messages_f, 42 )
            .match(
                [&]( rpc::void_return_type ) {},
                [&]( rpc::error err ) {
                        pretty_printer< std::stringstream > pp{};
                        pp << err;
                        FAIL() << pp.str();
                } );
}

TEST( rpc, bind )
{
        foo f;

        bind_test_wrapper bwp;

        bwp.bind< CALL_1 >( [&]( int i ) -> int {
                return f.call1_m( i );
        } );

        bwp.bind< CALL_2 >( [&]( float val1, int32_t val2 ) -> std::tuple< int, int > {
                return f.call2_m( val1, val2 );
        } );

        bwp.bind< CALL_3 >( [&]( int i ) -> void {
                f.call3_m( i );
        } );

        auto exchange_messages_f = [&]( const auto& msg ) {
                return bwp.on_message( msg );
        };

        using con = rpc::controller< direct_test_wrapper >;

        con::call< CALL_1 >( exchange_messages_f, 42 )
            .match(
                [&]( int r ) {
                        EXPECT_EQ( r, 84 );
                },
                [&]( auto ) {
                        FAIL() << "got an error";
                } );

        con::call< CALL_2 >( exchange_messages_f, 0.4f, 666u )
            .match(
                [&]( std::tuple< int, int > res ) {
                        auto [a, b] = res;
                        EXPECT_EQ( a, 266 );
                        EXPECT_EQ( b, 42 );
                },
                [&]( auto ) {
                        FAIL() << "got an error";
                } );

        con::call< CALL_3 >( exchange_messages_f, 42 )
            .match(
                [&]( rpc::void_return_type ) {},
                [&]( rpc::error err ) {
                        pretty_printer< std::stringstream > pp{};
                        pp << err;
                        FAIL() << pp.str();
                } );
}
