#include "emlabcpp/algorithm.h"
#include <ctime>
#include <iostream>
#include <string>

namespace em = emlabcpp;

int main(int, char *[]) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));

        // ---------------------------------------------------------------------------------------
        // datastructures used in the example

        std::tuple<int, std::string> tpl_data{42, "wololo"};
        std::vector<int>             vec_data{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 0};

        // ---------------------------------------------------------------------------------------
        // one of the two basic algorithms - for_each, the crucial property of both these is that
        // they have overload for both - begin/end containers and std::tuple-like data.

        em::for_each(tpl_data, [&](const auto &item) {
                std::cout << item << '\n';
        });

        em::for_each(vec_data, [&](int item) {
                std::cout << item << '\n';
        });

        // ---------------------------------------------------------------------------------------
        // seocnd of the two basic algorithms - find_if, again it exists in version for tuples and
        // containers. The tuple version returns index of item that matched the predicate.

        std::size_t index = em::find_if(tpl_data, [&](auto item) {
                return std::is_same_v<decltype(item), std::string>;
        });
        std::cout << "Item with std::string type is at position: " << index << '\n';

        auto iter = em::find_if(vec_data, [&](int i) {
                return i == 0;
        });
        std::cout << "Zero item is at pos: " << std::distance(iter, vec_data.begin()) << '\n';

        // ---------------------------------------------------------------------------------------
        // Basic set of handy functions contains `ignore(val)` to shutdown compilers error with
        // unused variable - just call ignore() over it, it does nothing.

        int unused_variable = 0;
        em::ignore(unused_variable);

        // ---------------------------------------------------------------------------------------
        // `sign(x)` is function that returns an int representing sign of x, that you can do to
        // implement more processing based on it, or multiply different value to mirror sign (such
        // as sign(x)*y). Keep in mind that it returns 0 for 0 input value.

        int sign_rand_val = (std::rand() % 3) - 1;
        switch (em::sign(sign_rand_val)) {
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

        float mapped = em::map_range(42, 0, 255, 0.0f, 1.0f);
        std::cout << "42 mapped from range 0-255 to range 0.-1. is: " << mapped << "\n";

        // ---------------------------------------------------------------------------------------
        // `almost_equal(a,b,e)` is used to check that `a` and `b` is close enough to each other, in
        // situations where equality is not expected or problematic (floats...). `e` is tolerance
        // value.

        if (em::almost_equal(0.0f, 0.1f, 0.5)) {
                std::cout << "Yaaaay, 0.1 is within a tolerance of 0.5 to 0.\n";
        }

        // ---------------------------------------------------------------------------------------
        // `tail` returns a view over provided container without the first item, handy for the code
        // that handles first item differently.

        for (int i : em::tail(vec_data)) {
                std::cout << i << '\n'; // note that `1` from the container is ignored
        }

        // ---------------------------------------------------------------------------------------
        // `init` returns a view over provided container without the last item, handy for the code
        // that handles last item differently.

        for (int i : em::init(vec_data)) {
                std::cout << i << '\n'; // note that `0` from the container is ignored
        }

        // ---------------------------------------------------------------------------------------
        // `min_max_elem` gives you abillity to find minimal amd maximal value of some calculation
        // applied to each item of container - such as min/max absolute value of integers in vector.
        //
        // There also exists min_elem and max_elem for calculation just part of it.

        em::min_max<int> mm_res = em::min_max_elem(vec_data, [](int val) {
                return em::abs(val);
        });
        std::cout << "Minimal absolute value of vec_data is " << mm_res.min << " and maximum is "
                  << mm_res.max << "\n";

        // ---------------------------------------------------------------------------------------
        // `sum` is function that can be used to '+=' data from container (where third argument is
        // optional default value) and apply function to them at the same time. Usefull for
        // calculation sums in math sense.

        int squared_sum = em::sum(vec_data, [](int val) {
                return val * val;
        });
        std::cout << "Squared sum of values from vector is: " << squared_sum << "\n";

        // ---------------------------------------------------------------------------------------
        // `accumulate` gets a container `c` and folds function `f`  over it with first value being
        // 's', this forms f(f(f(f(s,c[0]),c[1]),c[2])... chain Usefull for for implementing `sum`
        // or doing multiplication instead.

        int multiplied_vec = em::accumulate(vec_data, 1, [](int base, int val) {
                return base * val;
        });
        std::cout << "Value of all values in vector multiplied together is: " << multiplied_vec
                  << "\n";
}
