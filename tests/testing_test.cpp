
#include "emlabcpp/testing/reactor.h"

#include <gtest/gtest.h>

using namespace em = emlabcpp;

void simple_test_case( em::testing_record& rec )
{
        rec.fail();
}

struct my_test_fixture : em::testing_interface
{
        // disabling copy should be allowed
        my_test_fixture( const my_test_fixture& ) = delete;
        my_test_fixture& operator=( const my_test_fixture& ) = delete;

        void setup( em::testing_record& )
        {
                // setup i2c
        }

        void teardown( em::testing_record& )
        {
                // disable i2c
        }
}

struct my_test_case : my_test_fixture
{
        const std::array< uint8_t, 32 >& buffer;

        void run( em::testing_record& rec )
        {
                rec.log( buffer );

                rec.success();
        }
}

int main()
{
        em::testing_reactor rec;
        rec.register_callable( "simple test", simple_test_case );
        std::array< uint8_t, 32 > buffer;
        rec.register_callable( "simple lambda test", [&]( em::testing_record& rec ) {
                rec.log( buffer );

                rec.success();
        } );
        rec.register_test( "simple struct test", my_test_case{ buffer } );

        rec.register_callable( "complex lambda test", [&]( em::testing_record& rec ) {
                // propably something like em::static_vector<uint8_t,8>;
                using buffer = typename em::testing_record::arg_buffer;

                buffer data = rec.get_binary_arg();

                if ( data.size() > 6 ) {
                        rec.success();
                } else {
                        rec.fail();
                }
        } );

        rec.register_callable( "complex lambda test", [&]( em::testing_record& rec ) {
                using tpl                    = std::tuple< int32_t, uint8_t, uint8_t >;
                std::optional< tpl > opt_val = em::protocol_conv< tpl >( rec.get_binary_arg() );

                if ( !opt_val ) {
                        rec.log( "input arg to test is deserializable" );
                        return;
                }

                auto [bigval, x, y] = *opt_val;

                rec.expect( x > y );
        } );

        rec.register_test(
            "lambda amd fixture",
            em::testing_combine(
                [&]( em::testing_record& rec ) {
                        // check that i2c works

                        rec.expect( 1 > 0 );
                },
                my_testing_fixture{} ) );

        std::array< uint8_t, 64 > data_input;
        rec.spin( data_input );
}

