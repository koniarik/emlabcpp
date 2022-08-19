#include "emlabcpp/algorithm.h"

#include <ctime>
#include <iostream>
#include <list>
#include <string>

namespace em = emlabcpp;

int main( int, char*[] )
{
        // ---------------------------------------------------------------------------------------
        // algorithm.h is library of functions similar to standard <algorithm> library. The key
        // exception is that it contains more functions and the works with containers rather than
        // with iterators. When possible, the functions also can work over std::tuple-like objects.

        std::srand( static_cast< unsigned >( std::time( nullptr ) ) );

        // ---------------------------------------------------------------------------------------
        // data structures used in the examples

        std::tuple< int, std::string > tpl_data{ 42, "wololo" };
        std::vector< int >             vec_data{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0 };
        std::list< int >               list_data{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0 };

        // ---------------------------------------------------------------------------------------
        // `for_each(cont, f)` simply executes lambda `f`  over each item in container `cont`, the
        // crucial property is that container can be either begin/end container or std::tuple-like
        // container. This is used to implement algorithms that work over both types of containers.

        em::for_each( tpl_data, [&]( const auto& item ) {
                std::cout << item << '\n';
        } );

        em::for_each( vec_data, [&]( int item ) {
                std::cout << item << '\n';
        } );

        // ---------------------------------------------------------------------------------------
        // `find_if(cont,f)` is used to find first item in container `cont` that satisfies predicate
        // `f`. This returns index of item for std::tuple-like items and iterator for begin/end
        // containers.

        std::size_t index = em::find_if( tpl_data, [&]( auto item ) {
                return std::is_same_v< decltype( item ), std::string >;
        } );
        std::cout << "Item with std::string type is at position: " << index << '\n';

        auto iter = em::find_if( vec_data, [&]( int i ) {
                return i == 0;
        } );
        std::cout << "Zero item is at position: " << std::distance( iter, vec_data.begin() )
                  << '\n';

        // ---------------------------------------------------------------------------------------
        // `sign(x)` is used to indicate a sign of variables required for some algorithms. It can be
        // used to either explicitly control flow based on the sign or use it to propagate the sign
        // into a computation. Note that we consider 0 as a separate case.

        int sign_rand_val = ( std::rand() % 3 ) - 1;
        switch ( em::sign( sign_rand_val ) ) {
                case -1:
                        std::cout << "sign_rand_val is negative\n";
                        break;
                case 0:
                        std::cout << "sign_rand_val is zero\n";
                        break;
                case 1:
                        std::cout << "sign_rand_val is positive\n";
                        break;
        }

        // ---------------------------------------------------------------------------------------
        // `map_range(x,a,b,y,z)` is used to linearly convert values from one range into another.
        // Let's say your light sensor returns values in range 0-255 which actually just represents
        // 0-100% intensity.

        float mapped = em::map_range( 42, 0, 255, 0.0f, 1.0f );
        std::cout << "42 mapped from range 0-255 to range 0-1, is: " << mapped << '\n';

        // ---------------------------------------------------------------------------------------
        // `almost_equal(a,b,e)` is used to check that `a` and `b` is close enough to each other, in
        // situations where equality is not expected or problematic (floats...). `e` specifies
        // tolerable range

        if ( em::almost_equal( 0.0f, 0.1f, 0.5 ) ) {
                std::cout << "Yaaaay, 0.1 is within a tolerance of 0.5 to 0.\n";
        }

        // ---------------------------------------------------------------------------------------
        // `tail(cont)` returns a view over provided container without the first item, handy for the
        // code that handles first item differently.

        for ( int i : em::tail( vec_data ) ) {
                std::cout << i << '\n';  // note that `1` from the container is ignored
        }

        // ---------------------------------------------------------------------------------------
        // `init(cont)` returns a view over provided container without the last item, handy for the
        // code that handles last item differently.

        for ( int i : em::init( vec_data ) ) {
                std::cout << i << '\n';  // note that `0` from the container is ignored
        }

        // ---------------------------------------------------------------------------------------
        // `min_max_elem(cont, f)` is used to find minimal and maximal value of some
        // calculation applied to each item of container - such as min/max absolute value of
        // integers in vector.
        //
        // There also exists `min_elem` and `max_elem`.

        em::min_max< int > mm_res = em::min_max_elem( vec_data, []( int val ) {
                return std::abs( val );
        } );
        std::cout << "Minimal absolute value of vec_data is " << mm_res.min << " and maximum is "
                  << mm_res.max << '\n';

        // ---------------------------------------------------------------------------------------
        // `sum(cont, f)` returns sum of result of `f` applied over container `cont`, useful to
        // mirror sum in mathematical sense.

        int squared_sum = em::sum( vec_data, []( int val ) {
                return val * val;
        } );
        std::cout << "Squared sum of values from vector is: " << squared_sum << '\n';

        // ---------------------------------------------------------------------------------------
        // `accumulate(cont, f)` gets a container `c` and folds function `f`  over it with first
        // value being 's', this forms `f(f(f(f(s,c[0]),c[1]),c[2])...` chain. Useful for for
        // implementing `sum` or doing multiplication instead.

        int multiplied_vec = em::accumulate( vec_data, 1, []( int base, int val ) {
                return base * val;
        } );
        std::cout << "Value of all values in vector multiplied together is: " << multiplied_vec
                  << '\n';

        // ---------------------------------------------------------------------------------------
        // `avg(cont, f)` can be used to calculate average value of items, with function
        // pre-processing them.

        float avg_vec = em::avg( vec_data, [&]( int val ) {
                return static_cast< float >( val );
        } );
        std::cout << "Average value of items in vec_data is: " << avg_vec << '\n';

        // ---------------------------------------------------------------------------------------
        // for_cross_joint(a,b,f) applies `f` to all combinations of values, used for combinational
        // stuff and generation of data

        em::for_cross_joint( tpl_data, vec_data, []( auto tpl_val, int vec_val ) {
                std::cout << "tpl val: " << tpl_val << " vec val: " << vec_val << '\n';
        } );

        // ---------------------------------------------------------------------------------------
        // `any_of(cont, f)` returns true if the lambda evaluates at true to at least one item in
        // the container. This can be used for checking various properties over container, there is
        // also `none_of(cont, f)` and `all_of(cont, f)`

        bool has_zero_val = em::any_of( vec_data, []( int val ) {
                return val == 0;
        } );
        if ( has_zero_val ) {
                std::cout << "there is zero in vec_data \\o/ \n";
        }

        // ---------------------------------------------------------------------------------------
        // `equal(cont1, cont2)` compares content of containers for equality, this avoids the need
        // for same types of containers or values.

        bool content_is_equal = em::equal( vec_data, list_data );
        std::cout << "The statement that vec_data and list_data has equal content is: "
                  << ( content_is_equal ? "true" : "false" ) << '\n';

        // ---------------------------------------------------------------------------------------
        // `map_f(cont, f)` is used to generate new containers of data based on source container and
        // conversion function. You have to specify the output container as template argument. There
        // are also `map_f_to_a` instances, which returns std::array - avoids the need to specify
        // the return container in template.
        //
        // This is useful for situations like: for each item in dataset calculate X and store it in
        // a new container.

        auto mapped_vec = em::map_f< std::vector< int > >( vec_data, []( int val ) {
                return -val;
        } );
        for ( int v : mapped_vec ) {
                std::cout << "mapped val: " << v << '\n';
        }

        std::array< std::string, 11 > mapped_arr = em::map_f_to_a< 11 >( vec_data, []( auto item ) {
                return std::to_string( item );
        } );
        for ( const std::string& s : mapped_arr ) {
                std::cout << "mapped arr val: " << s << '\n';
        }

        // ---------------------------------------------------------------------------------------
        // as a shortcut for `map_f` functions, we have `convert_to<T>` and it's `T operator(U)`
        // which simply constructs T from inserted types.

        struct foo_conv
        {
                int i;
        };

        auto mapped_struct_vec =
            em::map_f< std::vector< foo_conv > >( vec_data, em::convert_to< foo_conv >{} );

        for ( foo_conv v : mapped_struct_vec ) {
                std::cout << "mapped float val: " << v.i << '\n';
        }

        // ---------------------------------------------------------------------------------------
        // `joined` is used for example to join strings with an delimiter

        std::vector< std::string > string_data{ "a", "b", "c", "d", "e" };

        std::cout << "joined string data: " << em::joined( string_data, std::string{ "," } )
                  << '\n';
}
