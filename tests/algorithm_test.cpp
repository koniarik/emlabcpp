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

#include "emlabcpp/algorithm.h"

#include "emlabcpp/quantity.h"

#include <gtest/gtest.h>
#include <list>

namespace emlabcpp
{

// NOLINTNEXTLINE
TEST( Algorithm, sign )
{
        EXPECT_EQ( sign( -5 ), -1 );
        EXPECT_EQ( sign( -0.1f ), -1 );
        EXPECT_EQ( sign( 0 ), 0 );
        EXPECT_EQ( sign( 1 ), 1 );
        EXPECT_EQ( sign( 10 ), 1 );

        int const i = 10;
        EXPECT_EQ( sign( i ), 1 );
}

TEST( Algorithm, ceil_to )
{
        EXPECT_EQ( ceil_to( 1, 5 ), 5 );
        EXPECT_EQ( ceil_to( 2, 5 ), 5 );
        EXPECT_EQ( ceil_to( 4, 5 ), 5 );
        EXPECT_EQ( ceil_to( 5, 5 ), 5 );
}

// NOLINTNEXTLINE
TEST( Algorithm, map_range )
{
        EXPECT_EQ( map_range( 5, 0, 10, 0, 100 ), 50 );
        EXPECT_EQ( map_range( 5, 0, 10, 10, 20 ), 15 );
        EXPECT_EQ( map_range( 5, 0, 10, 10, 0 ), 5 );
        EXPECT_EQ( map_range( 10, 0, 10, 10, 0 ), 0 );
        EXPECT_EQ( map_range( 0, 0, 10, 10, 0 ), 10 );
}

// NOLINTNEXTLINE
TEST( Algorithm, almost_equal )
{
        EXPECT_TRUE( almost_equal( 0, 0 ) );
        EXPECT_FALSE( almost_equal( 0, 1 ) );
        EXPECT_TRUE( almost_equal( 0, 1, 2 ) );
}

// NOLINTNEXTLINE
TEST( Algorithm, tail )
{
        std::vector< int > test = { 1, 2, 3 };
        EXPECT_EQ( tail( test ).size(), std::size_t{ 2 } );
        EXPECT_EQ( tail( test ).front(), 2 );
        EXPECT_EQ( tail( test ).back(), 3 );
        EXPECT_EQ( tail( test, 2 ).size(), std::size_t{ 1 } );
        EXPECT_EQ( tail( test, 2 ).front(), 3 );
}

// NOLINTNEXTLINE
TEST( Algorithm, init )
{
        std::vector< int > test = { 1, 2, 3 };
        EXPECT_EQ( init( test ).size(), std::size_t{ 2 } );
        EXPECT_EQ( init( test ).front(), 1 );
        EXPECT_EQ( init( test ).back(), 2 );
        EXPECT_EQ( init( test, 2 ).size(), std::size_t{ 1 } );
        EXPECT_EQ( init( test, 2 ).front(), 1 );
}

// NOLINTNEXTLINE
TEST( Algorithm, find_if_bool_tup )
{
        std::tuple< bool, bool, bool > tup = { false, true, false };
        EXPECT_EQ( find_if( tup ), std::size_t{ 1 } );

        tup = { false, false, false };
        EXPECT_EQ( find_if( tup ), cont_size( tup ) );

        tup = { false, false, false };
        EXPECT_EQ(
            find_if(
                tup,
                []( bool v ) {
                        return !v;
                } ),
            0 );

        tup = { false, true, false };
        EXPECT_EQ( find_if( std::move( tup ) ), std::size_t{ 1 } );
}

// NOLINTNEXTLINE
TEST( Algorithm, find_if_int_tup )
{
        std::tuple< int, int, int > const tup2 = { -1, 0, 1 };
        std::size_t const                 i    = find_if( tup2, [&]( int i ) {
                return i > 0;
        } );
        EXPECT_EQ( i, 2u );
}

// NOLINTNEXTLINE
TEST( Algorithm, find_if_vector )
{
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

// NOLINTNEXTLINE
TEST( Algorithm, find )
{
        std::tuple< int, int, int > const tup2 = { -1, 0, 1 };
        std::size_t const                 i    = find( tup2, 1 );
        EXPECT_EQ( i, 2u );

        std::vector< int > vec1 = { -1, 0, 1 };

        auto iter = find( vec1, 1 );
        EXPECT_EQ( std::distance( vec1.begin(), iter ), 2u );
}

// NOLINTNEXTLINE
TEST( Algorithm, contains )
{
        std::vector< int > const vec = { 0, 1, 2, 42, 3, 4, 5 };
        EXPECT_TRUE( contains( vec, 42 ) );
        EXPECT_FALSE( contains( vec, 666 ) );

        std::tuple< int, int > const tpl = { -1, 1 };
        EXPECT_TRUE( contains( tpl, 1 ) );
        EXPECT_FALSE( contains( tpl, 0 ) );
}

// NOLINTNEXTLINE
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

// NOLINTNEXTLINE
TEST( Algorithm, min_max_elem )
{
        min_max< int > res = min_max_elem( std::vector< int >{ 1, 2, 3 }, [&]( int i ) {
                return i * 2;
        } );
        EXPECT_EQ( res.min(), 2 );
        EXPECT_EQ( res.max(), 6 );
        res = min_max_elem( std::tuple< int, int, int >{ 1, 2, 3 }, [&]( int i ) {
                return i * 2;
        } );
        EXPECT_EQ( res.min(), 2 );
        EXPECT_EQ( res.max(), 6 );
}

// NOLINTNEXTLINE
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

// NOLINTNEXTLINE
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

// NOLINTNEXTLINE
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

// NOLINTNEXTLINE
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

// NOLINTNEXTLINE
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

// NOLINTNEXTLINE
TEST( Algorithm, avg )
{
        std::size_t res = avg( std::vector< std::size_t >{ 1, 2, 3 } );
        EXPECT_EQ( res, std::size_t( 2 ) );
        res = avg( std::vector< std::size_t >{ 1, 2, 3 }, [&]( std::size_t i ) {
                return i * 2;
        } );
        EXPECT_EQ( res, std::size_t( 4 ) );
        res = avg( std::tuple< std::size_t, std::size_t, std::size_t >{ 1, 2, 3 } );
        EXPECT_EQ( res, std::size_t( 2 ) );

        using test_type      = tagged_quantity< struct avg_test_type_tag, std::size_t >;
        test_type const res2 = avg( std::vector< test_type >{ test_type{ 0 }, test_type{ 10 } } );
        EXPECT_EQ( res2, test_type{ 5u } );
}

// NOLINTNEXTLINE
TEST( Algorithm, variance )
{
        std::size_t res = variance( std::vector< std::size_t >{ 1, 2, 3, 4 } );
        EXPECT_EQ( res, std::size_t( 1 ) );
        res = variance(
            std::tuple< std::size_t, std::size_t, std::size_t, std::size_t >{ 1, 2, 3, 4 } );
        EXPECT_EQ( res, std::size_t( 1 ) );
        float const fres = variance( std::vector< float >{ 1.f, 2.f, 3.f, 4.f } );
        EXPECT_EQ( fres, 1.25f );

        using test_type        = tagged_quantity< struct variance_test_type_tag, std::size_t >;
        std::size_t const res2 = variance(
            std::vector< test_type >{ test_type{ 0 }, test_type{ 10 } }, [&]( test_type v ) {
                    return *v;
            } );
        EXPECT_EQ( res2, 25u );
}

// NOLINTNEXTLINE
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

// NOLINTNEXTLINE
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

// NOLINTNEXTLINE
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

// NOLINTNEXTLINE
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

// NOLINTNEXTLINE
TEST( Algorithm, equal )
{

        bool res = equal( std::array< int, 3 >{ 5, 6, 7 }, std::vector< int >{ 5, 6, 7 } );
        EXPECT_TRUE( res );

        res = equal( std::list< int >{ 5, 6, 7 }, std::vector< int >{ 5, 6 } );
        EXPECT_FALSE( res );

        res = equal( std::list< int >{ 5, 6, 7 }, std::vector< int >{ 7, 6, 5 } );
        EXPECT_FALSE( res );
}

// NOLINTNEXTLINE
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

        float const mapped_sum = sum( map_f< std::list< float > >( idata, [&]( int i ) {
                return float( i ) * 1.5f;
        } ) );
        EXPECT_EQ( mapped_sum, 15.f );

        std::tuple< int, int, int, int > tdata{ 1, 2, 3, 4 };
        is_equal = equal( map_f< std::vector< int > >( tdata ), idata );
        EXPECT_TRUE( is_equal );

        std::map< int, std::string > const expected{
            { 1, "1" }, { 2, "2" }, { 3, "3" }, { 4, "4" } };
        auto converted = map_f< std::map< int, std::string > >( idata, []( int i ) {
                return std::make_pair( i, std::to_string( i ) );
        } );
        EXPECT_EQ( converted, expected );

        try {
                converted = map_f< std::map< int, std::string > >( idata, []( int i ) {
                        throw std::runtime_error{ "testing error" };

                        return std::make_pair( i, std::to_string( i ) );
                } );
        }
        catch ( std::runtime_error& e ) {
                EXPECT_EQ( std::string{ e.what() }, std::string{ "testing error" } );
        }
}

// NOLINTNEXTLINE
TEST( Algorithm, map_f_to_a_vector )
{
        std::vector< int > idata{ 1, 2, 3, 4 };

        bool is_equal = equal( map_f_to_a< 4 >( idata ), idata );
        EXPECT_TRUE( is_equal );

        std::vector< int > const goal_data{ 2, 4, 6, 8 };
        is_equal = equal(
            map_f_to_a< 4 >(
                std::move( idata ),
                []( int i ) {
                        return i * 2;
                } ),
            goal_data );
        EXPECT_TRUE( is_equal );
}

TEST( Algorithm, map_f_to_a_array )
{
        std::array< int, 4 > idata2{ 1, 2, 3, 4 };

        bool const is_equal = equal( map_f_to_a( idata2 ), idata2 );
        EXPECT_TRUE( is_equal );
}

TEST( Algorithm, map_f_to_a_tuple )
{
        std::tuple< int, float, int > tup1     = { -1, 0.f, 1 };
        std::array< std::string, 3 >  tup1_res = map_f_to_a( tup1, []( auto v ) {
                return std::to_string( v );
        } );

        EXPECT_EQ( tup1_res[0], "-1" );
        EXPECT_EQ( tup1_res[1], "0.000000" );
        EXPECT_EQ( tup1_res[2], "1" );

        std::array< bool, 3 > const arr1_res = map_f_to_a( std::move( tup1 ), []( auto v ) {
                return v != 0;
        } );

        std::array< bool, 3 > const expected = { true, false, true };
        EXPECT_EQ( arr1_res, expected );
}

struct convert_test
{
        int val;
};

// NOLINTNEXTLINE
TEST( Algorithm, convert_to )
{
        std::vector< int > const idata{ 1, 2, 3, 4 };

        auto mapped = map_f< std::vector< convert_test > >( idata, convert_to< convert_test >{} );
        EXPECT_EQ( mapped[0].val, 1 );
        EXPECT_EQ( mapped[3].val, 4 );
}

// NOLINTNEXTLINE
TEST( Algorithm, joined )
{
        std::vector< std::string > const idata{ "ab", "cd", "ef" };
        std::string                      msg = joined( idata, std::string{ "|" } );

        EXPECT_EQ( msg, "ab|cd|ef" );

        msg = joined( std::vector< std::string >{}, std::string{ "|" } );
        EXPECT_EQ( msg, "" );

        std::vector< int > iidata{ 1, 2, 3, 4 };
        int                res = joined( iidata, 1 );

        EXPECT_EQ( res, 13 );

        iidata = { 1 };
        res    = joined( iidata, 0, []( int j ) {
                return j + 1;
        } );
        EXPECT_EQ( res, 2 );
}

// NOLINTNEXTLINE
TEST( Algorithm, for_each_index )
{
        std::vector< std::size_t > idata{ 1, 2, 3, 4 };
        std::vector< std::size_t > odata{};

        for_each_index< 4 >( [&]< std::size_t i >() {
                odata.push_back( idata[i] );
        } );

        EXPECT_EQ( idata, odata );
}

// NOLINTNEXTLINE
TEST( Algorithm, find_if_index )
{
        std::vector< std::size_t > idata{ 1, 2, 3, 4 };

        std::size_t const i = find_if_index< 4 >( [&]< std::size_t i >() {
                return idata[i] == 3;
        } );

        EXPECT_EQ( i, 2 );
}

// NOLINTNEXTLINE
TEST( Algorithm, until_index )
{
        std::vector< std::size_t > idata{ 1, 2, 3, 4 };
        std::vector< std::size_t > odata{};

        until_index< 8 >( [&]< std::size_t i >() {
                if ( odata.size() == idata.size() )
                        return true;
                odata.push_back( idata[i] );
                return false;
        } );

        EXPECT_EQ( idata, odata );
}

// NOLINTNEXTLINE
TEST( Algorithm, select_index )
{
        std::vector< std::size_t > idata{ 1, 2, 3, 4 };
        for ( std::size_t const i : idata ) {
                bounded const     b = bounded< std::size_t, 0, 3 >::make( i - 1 ).value();
                std::size_t const j = select_index( b, [&]< std::size_t i >() -> std::size_t {
                        return idata[i];
                } );
                EXPECT_EQ( i, j ) << "bounded val: " << b;
        }
}

TEST( Algorithm, bytes )
{
        std::array< std::byte, 3 > data = bytes( 1, 2, 3 );
        EXPECT_EQ( data[0], std::byte{ 1 } );
        EXPECT_EQ( data[1], std::byte{ 2 } );
        EXPECT_EQ( data[2], std::byte{ 3 } );
}

TEST( Algorithm, merge_arrays )
{
        std::array< int, 2 > d1  = { 42, 666 };
        std::array< int, 3 > d2  = { 1, 2, 3 };
        std::array< int, 5 > res = merge_arrays( d1, d2 );

        std::array< int, 5 > expected = { 42, 666, 1, 2, 3 };
        EXPECT_EQ( res, expected );
}

TEST( Algorithm, filled )
{
        std::array< int, 5 > d        = filled< 5 >( 42 );
        std::array< int, 5 > expected = { 42, 42, 42, 42, 42 };
        EXPECT_EQ( d, expected );
}

}  // namespace emlabcpp
