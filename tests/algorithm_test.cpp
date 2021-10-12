#include "emlabcpp/algorithm.h"

#include <gtest/gtest.h>
#include <list>

using namespace emlabcpp;

TEST( Algorithm, max )
{
        EXPECT_EQ( max( 5, 10 ), 10 );
        EXPECT_EQ( max( -5, -10 ), -5 );
}

TEST( Algorithm, min )
{
        EXPECT_EQ( min( 5, 10 ), 5 );
        EXPECT_EQ( min( -5, -10 ), -10 );
}

TEST( Algorithm, sign )
{
        EXPECT_EQ( sign( -5 ), -1 );
        EXPECT_EQ( sign( -0.1f ), -1 );
        EXPECT_EQ( sign( 0 ), 0 );
        EXPECT_EQ( sign( 1 ), 1 );
        EXPECT_EQ( sign( 10 ), 1 );

        int i = 10;
        EXPECT_EQ( sign( i ), 1 );
}

TEST( Algorithm, map_range )
{
        EXPECT_EQ( map_range( 5, 0, 10, 0, 100 ), 50 );
        EXPECT_EQ( map_range( 5, 0, 10, 10, 20 ), 15 );
        EXPECT_EQ( map_range( 5, 0, 10, 10, 0 ), 5 );
        EXPECT_EQ( map_range( 10, 0, 10, 10, 0 ), 0 );
        EXPECT_EQ( map_range( 0, 0, 10, 10, 0 ), 10 );
}

TEST( Algorithm, uncurry )
{
        auto test_tuple = std::make_tuple( 1, 2.f, 3. );
        uncurry( [&]( int a, float b, double c ) {
                EXPECT_EQ( a, std::get< int >( test_tuple ) );
                EXPECT_EQ( b, std::get< float >( test_tuple ) );
                EXPECT_EQ( c, std::get< double >( test_tuple ) );
        } )( test_tuple );
}

TEST( Algorithm, tail )
{
        std::vector< int > test = { 1, 2, 3 };
        EXPECT_EQ( tail( test ).size(), std::size_t{ 2 } );
        EXPECT_EQ( tail( test ).front(), 2 );
        EXPECT_EQ( tail( test ).back(), 3 );
        EXPECT_EQ( tail( test, 2 ).size(), std::size_t{ 1 } );
        EXPECT_EQ( tail( test, 2 ).front(), 3 );
}

TEST( Algorithm, init )
{
        std::vector< int > test = { 1, 2, 3 };
        EXPECT_EQ( init( test ).size(), std::size_t{ 2 } );
        EXPECT_EQ( init( test ).front(), 1 );
        EXPECT_EQ( init( test ).back(), 2 );
        EXPECT_EQ( init( test, 2 ).size(), std::size_t{ 1 } );
        EXPECT_EQ( init( test, 2 ).front(), 1 );
}

TEST( Algorithm, find_if )
{
        std::tuple< bool, bool, bool > tup = { false, true, false };
        EXPECT_EQ( find_if( tup ), std::size_t{ 1 } );
        tup = { false, false, false };
        EXPECT_EQ( find_if( tup ), cont_size( tup ) );

        std::tuple< int, int, int > tup2 = { -1, 0, 1 };
        std::size_t                 i    = find_if( tup2, [&]( int i ) {
                return i > 0;
        } );
        EXPECT_EQ( i, std::size_t{ 2 } );

        std::vector< bool > vec = { false, true, false };
        EXPECT_EQ( find_if( vec ), ++vec.begin() );
        vec = { false, false, false };
        EXPECT_EQ( find_if( vec ), vec.end() );

        std::vector< int > vec2 = { -1, 0, 1 };
        auto               iter = find_if( vec2, [&]( int i ) {
                return i > 0;
        } );
        EXPECT_EQ( iter, --vec2.end() );
}

TEST( Algorithm, for_each )
{
        std::vector< int > data{ 1, 2, 3 };
        int                tmp = 0;
        for_each( data, [&]( int i ) {
                tmp += i;
        } );
        EXPECT_EQ( tmp, 6 );

        std::size_t j = 0;
        for_each( std::tuple< int, int, int >{ 1, 2, 3 }, [&]( int i ) {
                EXPECT_EQ( data[j], i );
                j++;
        } );
}

TEST( Algorithm, min_max_elem )
{
        min_max< int > res = min_max_elem( std::vector< int >{ 1, 2, 3 }, [&]( int i ) {
                return i * 2;
        } );
        EXPECT_EQ( res.min, 2 );
        EXPECT_EQ( res.max, 6 );
        res = min_max_elem( std::tuple< int, int, int >{ 1, 2, 3 }, [&]( int i ) {
                return i * 2;
        } );
        EXPECT_EQ( res.min, 2 );
        EXPECT_EQ( res.max, 6 );
}

TEST( Algorithm, max_elem )
{
        int res = max_elem( std::vector< int >{ 1, 2, 3 }, [&]( int i ) {
                return i * 2;
        } );
        EXPECT_EQ( res, 6 );
        res = max_elem( std::tuple< int, int, int >{ 1, 2, 3 }, [&]( int i ) {
                return i * 2;
        } );
        EXPECT_EQ( res, 6 );
}

TEST( Algorithm, min_elem )
{
        int res = min_elem( std::vector< int >{ 1, 2, 3 }, [&]( int i ) {
                return i * 2;
        } );
        EXPECT_EQ( res, 2 );
        res = min_elem( std::tuple< int, int, int >{ 1, 2, 3 }, [&]( int i ) {
                return i * 2;
        } );
        EXPECT_EQ( res, 2 );
}

TEST( Algorithm, count )
{
        std::size_t res = count( std::vector< int >{ 1, 2, 3 }, [&]( int i ) {
                return i > 1;
        } );
        EXPECT_EQ( res, std::size_t{ 2 } );
        res = count( std::tuple< int, int, int >{ 1, 2, 3 }, [&]( int i ) {
                return i > 1;
        } );
        EXPECT_EQ( res, std::size_t{ 2 } );
}

TEST( Algorithm, sum )
{
        int res = sum( std::vector< int >{ 1, 2, 3 }, [&]( int i ) {
                return i * 2;
        } );
        EXPECT_EQ( res, 12 );
        res = sum( std::tuple< int, int, int >{ 1, 2, 3 }, [&]( int i ) {
                return i * 2;
        } );
        EXPECT_EQ( res, 12 );
}

TEST( Algorithm, accumulate )
{
        int res = accumulate( std::vector< int >{ 1, 2, 3 }, -6, [&]( int i, int j ) {
                return i + j;
        } );
        EXPECT_EQ( res, 0 );
        res = accumulate( std::tuple< int, int, int >{ 1, 2, 3 }, -6, [&]( int i, int j ) {
                return i + j;
        } );
        EXPECT_EQ( res, 0 );
}

TEST( Algorithm, avg )
{
        std::size_t res = avg( std::vector< std::size_t >{ 1, 2, 3 } );
        EXPECT_EQ( res, std::size_t( 2 ) );
        res = avg( std::tuple< std::size_t, std::size_t, std::size_t >{ 1, 2, 3 } );
        EXPECT_EQ( res, std::size_t( 2 ) );
}

TEST( Algorithm, variance )
{
        std::size_t res = variance( std::vector< std::size_t >{ 1, 2, 3, 4 } );
        EXPECT_EQ( res, std::size_t( 1 ) );
        res = variance(
            std::tuple< std::size_t, std::size_t, std::size_t, std::size_t >{ 1, 2, 3, 4 } );
        EXPECT_EQ( res, std::size_t( 1 ) );
        float fres = variance( std::vector< float >{ 1.f, 2.f, 3.f, 4.f } );
        EXPECT_EQ( fres, 1.25f );
}

TEST( Algorithm, for_cross_joint )
{
        float res = 0;
        for_cross_joint(
            std::tuple< int, int, int >{ 1, 2, 3 },
            std::vector< float >{ 4.f, 5.f, 6.f },
            [&]( int i, float f ) {
                    res += float( i ) * f;
            } );
        EXPECT_EQ( res, 90.f );
}

TEST( Algorithm, any_of )
{
        bool res = any_of( std::tuple< int, int, int >{ 1, 2, 3 }, []( int i ) {
                return i == 2;
        } );
        EXPECT_TRUE( res );
        res = any_of( std::tuple< int, int, int >{ 1, 2, 3 }, []( int i ) {
                return i == 0;
        } );
        EXPECT_FALSE( res );

        res = any_of( std::vector< int >{ 1, 2, 3 }, []( int i ) {
                return i == 2;
        } );
        EXPECT_TRUE( res );
        res = any_of( std::vector< int >{ 1, 2, 3 }, []( int i ) {
                return i == 0;
        } );
        EXPECT_FALSE( res );
}

TEST( Algorithm, none_of )
{
        bool res = none_of( std::tuple< int, int, int >{ 1, 2, 3 }, []( int i ) {
                return i == 2;
        } );
        EXPECT_FALSE( res );
        res = none_of( std::tuple< int, int, int >{ 1, 2, 3 }, []( int i ) {
                return i == 0;
        } );
        EXPECT_TRUE( res );

        res = none_of( std::vector< int >{ 1, 2, 3 }, []( int i ) {
                return i == 2;
        } );
        EXPECT_FALSE( res );
        res = none_of( std::vector< int >{ 1, 2, 3 }, []( int i ) {
                return i == 0;
        } );
        EXPECT_TRUE( res );
}

TEST( Algorithm, all_of )
{
        bool res = all_of( std::tuple< int, int, int >{ 1, 2, 3 }, []( int i ) {
                return i == 2;
        } );
        EXPECT_FALSE( res );
        res = all_of( std::tuple< int, int, int >{ 1, 2, 3 }, []( int i ) {
                return i > 0;
        } );
        EXPECT_TRUE( res );

        res = all_of( std::vector< int >{ 1, 2, 3 }, []( int i ) {
                return i == 2;
        } );
        EXPECT_FALSE( res );
        res = all_of( std::vector< int >{ 1, 2, 3 }, []( int i ) {
                return i > 0;
        } );
        EXPECT_TRUE( res );
}

TEST( Algorithm, equal )
{

        bool res = equal( std::array< int, 3 >{ 5, 6, 7 }, std::vector< int >{ 5, 6, 7 } );
        EXPECT_TRUE( res );

        res = equal( std::list< int >{ 5, 6, 7 }, std::vector< int >{ 5, 6 } );
        EXPECT_FALSE( res );

        res = equal( std::list< int >{ 5, 6, 7 }, std::vector< int >{ 7, 6, 5 } );
        EXPECT_FALSE( res );
}

TEST( Algorithm, map_f )
{
        std::vector< int > idata{ 1, 2, 3, 4 };

        bool is_equal = equal( map_f< std::vector< int > >( idata ), idata );
        EXPECT_TRUE( is_equal );

        is_equal = equal( map_f< std::list< int > >( idata ), idata );
        EXPECT_TRUE( is_equal );

        is_equal = equal( map_f< std::array< int, 4 > >( idata ), idata );
        auto res = map_f< std::array< int, 4 > >( idata );
        EXPECT_TRUE( is_equal ) << view{ res } << "," << view{ idata };

        float mapped_sum = sum( map_f< std::list< float > >( idata, [&]( int i ) {
                return float( i ) * 1.5f;
        } ) );
        EXPECT_EQ( mapped_sum, 15.f );

        std::tuple< int, int, int, int > tdata{ 1, 2, 3, 4 };
        is_equal = equal( map_f< std::vector< int > >( tdata ), idata );
        EXPECT_TRUE( is_equal );
}

TEST( Algorithm, map_f_to_a )
{
        std::vector< int > idata{ 1, 2, 3, 4 };

        bool is_equal = equal( map_f_to_a< 4 >( idata ), idata );
        EXPECT_TRUE( is_equal );

        std::array< int, 4 > idata2{ 1, 2, 3, 4 };

        is_equal = equal( map_f_to_a( idata2 ), idata2 );
        EXPECT_TRUE( is_equal );
}

struct convert_test
{
        int val;
};

TEST( Algorithm, convert_to )
{
        std::vector< int > idata{ 1, 2, 3, 4 };

        auto mapped = map_f< std::vector< convert_test > >( idata, convert_to< convert_test >{} );
        EXPECT_EQ( mapped[0].val, 1 );
        EXPECT_EQ( mapped[3].val, 4 );
}

TEST( Algorithm, joined )
{
        std::vector< std::string > idata{ "ab", "cd", "ef" };
        std::string                msg = joined( idata, std::string{ "|" } );

        EXPECT_EQ( msg, "ab|cd|ef" );
}

TEST( Algorithm, for_each_index )
{
        std::vector< std::size_t > idata{ 1, 2, 3, 4 };
        std::vector< std::size_t > odata{};

        for_each_index< 4 >( [&]< std::size_t i >() {
                odata.push_back( idata[i] );
        } );

        EXPECT_EQ( idata, odata );
}

TEST( Algorithm, until_index )
{
        std::vector< std::size_t > idata{ 1, 2, 3, 4 };
        std::vector< std::size_t > odata{};

        until_index< 8 >( [&]< std::size_t i >() {
                if ( odata.size() == idata.size() ) {
                        return true;
                }
                odata.push_back( idata[i] );
                return false;
        } );

        EXPECT_EQ( idata, odata );
}

TEST( Algorithm, select_index )
{
        std::vector< std::size_t > idata{ 1, 2, 3, 4 };
        for ( std::size_t i : idata ) {
                bounded     b = *bounded< std::size_t, 0, 3 >::make( i - 1 );
                std::size_t j = select_index( b, [&]< std::size_t i >() {
                        return idata[i];
                } );
                EXPECT_EQ( i, j ) << "bounded val: " << b;
        }
}
