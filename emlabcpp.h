#include <cmath>
#include <cstdlib>
#include <tuple>

#include <array>
#include <tuple>
#include <type_traits>

namespace emlabcpp {

// ------------------------------------------------------------------------------------------------
// iterator_of

template <typename Container>
struct iterator_of {
        using type = decltype(std::declval<std::remove_reference_t<Container>>().begin());
};

template <typename Container>
using iterator_of_t = typename iterator_of<Container>::type;

// ------------------------------------------------------------------------------------------------
// is_view

namespace detail {
template <typename>
struct is_view_impl : std::false_type {};
} // namespace detail

template <typename T>
constexpr bool is_view_v = detail::is_view_impl<std::decay_t<T>>::value;

// ------------------------------------------------------------------------------------------------
// are_same

template <typename...>
struct are_same;

template <typename T, typename... Ts>
struct are_same<T, Ts...> : std::conjunction<std::is_same<T, Ts>...> {};

template <>
struct are_same<> : std::true_type {};

template <typename... Ts>
constexpr bool are_same_v = are_same<Ts...>::value;

// ------------------------------------------------------------------------------------------------
// tuple_has_type

template <typename T, typename Tuple>
struct tuple_has_type;

template <typename T, typename... Us>
struct tuple_has_type<T, std::tuple<Us...>> : std::disjunction<std::is_same<T, Us>...> {};

template <typename T, typename... Us>
constexpr bool tuple_has_type_v = tuple_has_type<T, Us...>::value;

// ------------------------------------------------------------------------------------------------
// is_std_tuple

template <typename>
struct is_std_tuple : std::false_type {};

template <typename... T>
struct is_std_tuple<std::tuple<T...>> : std::true_type {};

template <typename T>
constexpr bool is_std_tuple_v = is_std_tuple<std::decay_t<T>>::value;

// ------------------------------------------------------------------------------------------------
// is_std_array

template <typename>
struct is_std_array : std::false_type {};

template <typename T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};

template <typename T>
constexpr bool is_std_array_v = is_std_array<std::decay_t<T>>::value;

// ------------------------------------------------------------------------------------------------
// static_size

template <typename>
struct static_size;

template <typename T, std::size_t N>
struct static_size<std::array<T, N>> {
        static constexpr std::size_t value = N;
};

template <typename... Ts>
struct static_size<std::tuple<Ts...>> {
        static constexpr std::size_t value = std::tuple_size_v<std::tuple<Ts...>>;
};

template <typename T>
constexpr std::size_t static_size_v = static_size<std::decay_t<T>>::value;

// ------------------------------------------------------------------------------------------------
// has_static_size

template <typename T>
struct has_static_size {
        template <typename U, typename = decltype(static_size<std::decay_t<U>>::value)>
        static std::true_type test(int);

        template <typename>
        static std::false_type test(...);

        static constexpr bool value = decltype(test<T>(0))::value;
};

template <typename T>
constexpr bool has_static_size_v = has_static_size<std::decay_t<T>>::value;

// ------------------------------------------------------------------------------------------------
// has_push_back

template <typename T>
struct has_push_back {

        template <typename U, typename = decltype(std::declval<U>().push_back(
                                  std::declval<typename U::value_type>()))>
        static std::true_type test(int);

        template <typename>
        static std::false_type test(...);

        static constexpr bool value = decltype(test<T>(0))::value;
};

template <typename T>
constexpr bool has_push_back_v = has_push_back<std::decay_t<T>>::value;

// ------------------------------------------------------------------------------------------------
// mapped

template <typename Container, typename UnaryFunction>
struct mapped {
        using type =
            decltype(std::declval<UnaryFunction>()(*std::begin(std::declval<Container>())));
};

template <typename UnaryFunction, typename T, typename... Ts>
struct mapped<std::tuple<T, Ts...>, UnaryFunction> {
        using type = decltype(std::declval<UnaryFunction>()(std::declval<T>()));
};

template <typename UnaryFunction, typename T, typename... Ts>
struct mapped<std::tuple<T, Ts...> &, UnaryFunction> {
        T item;
        using type = decltype(std::declval<UnaryFunction>()(item));
};

template <typename UnaryFunction, typename T, typename... Ts>
struct mapped<const std::tuple<T, Ts...> &, UnaryFunction> {
        using type = decltype(std::declval<UnaryFunction>()(std::declval<T>()));
};
template <typename UnaryFunction, typename T, typename... Ts>
struct mapped<std::tuple<T, Ts...> &&, UnaryFunction> {
        using type = decltype(std::declval<UnaryFunction>()(std::declval<T>()));
};

template <typename Container, typename UnaryFunction>
using mapped_t = typename mapped<Container, UnaryFunction>::type;

} // namespace emlabcpp

#include <tuple>

namespace emlabcpp {
template <typename Container, typename UnaryFunction>
constexpr std::enable_if_t<!is_std_tuple_v<Container>, void> for_each(Container &&    cont,
                                                                      UnaryFunction &&f);

template <typename Tuple, typename UnaryFunction>
constexpr std::enable_if_t<is_std_tuple_v<Tuple>, void> for_each(Tuple &&t, UnaryFunction &&f);

namespace detail {
template <typename... Args, typename UnaryFunction, std::size_t... Idx>
[[nodiscard]] constexpr std::size_t find_if_impl(const std::tuple<Args...> &t, UnaryFunction &&f,
                                                 std::index_sequence<Idx...>) {
        std::size_t res = sizeof...(Args);
        auto        ff  = [&](const auto &item, std::size_t i) {
                if (f(item)) {
                        res = i;
                        return true;
                }
                return false;
        };

        (ff(std::get<Idx>(t), Idx) || ...);

        return res;
}
template <typename Tuple, typename UnaryFunction, std::size_t... Idx>
constexpr void for_each_impl(Tuple &&t, UnaryFunction &&f, std::index_sequence<Idx...>) {
        (f(std::get<Idx>(std::forward<Tuple>(t))), ...);
}
template <typename T, std::size_t N, typename Container, typename UnaryFunction>
[[nodiscard]] inline std::array<T, N> map_f_to_a_impl(Container &&cont, UnaryFunction &&f) {
        std::array<T, N> res;
        std::size_t      i = 0;
        emlabcpp::for_each(std::forward<Container>(cont),
                           [&](auto &&item) { //
                                   res[i] = f(std::forward<decltype(item)>(item));
                                   ++i;
                           });
        return res;
}
} // namespace detail
} // namespace emlabcpp

namespace emlabcpp {

// Generic class to represent view of some container
//
template <typename Iterator>
class view {
        Iterator begin_;
        Iterator end_;

      public:
        // standard public usings for container
        using value_type = typename std::remove_reference_t<decltype(*std::declval<Iterator>())>;
        using reverse_iterator = std::reverse_iterator<Iterator>;
        using iterator         = Iterator;

        constexpr view() = default;

        // constructor from Container, uses begin/end of the container
        template <typename Container>
        constexpr view(Container &cont) : begin_(std::begin(cont)), end_(std::end(cont)) {}

        // constructor from the iterators that should internally be stored
        constexpr view(Iterator begin, Iterator end)
            : begin_(std::move(begin)), end_(std::move(end)) {}

        // Start of the dataset
        constexpr Iterator begin() const { return begin_; }

        // Past the end iterator
        constexpr Iterator end() const { return end_; }

        // Access to i-th element in the range, expects Iterator::operator[]
        constexpr decltype(auto) operator[](std::size_t i) const { return begin_[i]; }

        // Returns iterator to the last element that goes in reverse
        constexpr reverse_iterator rbegin() const { return reverse_iterator{end_}; }

        // Returns iterator to the element before first element, that can go in
        // reverse
        constexpr reverse_iterator rend() const { return reverse_iterator{begin_}; }

        // Size of the view over dataset uses std::distance() to tell the size
        constexpr std::size_t size() const {
                return static_cast<std::size_t>(std::distance(begin(), end()));
        }

        // View is empty if both iterators are equal
        constexpr bool empty() const { return begin() == end(); }

        // Returns first value of the range
        constexpr const value_type &front() const { return *begin_; }

        // Returns last value of the range
        constexpr const value_type &back() const { return *std::prev(end_); }
};

// The container deduction guide uses iterator_of_t
template <typename Container>
view(Container &cont)->view<iterator_of_t<Container>>;

// Support for our deduction guide to types - is_view_v
template <typename Iter>
struct detail::is_view_impl<view<Iter>> : std::true_type {};

// Creates view over 'n' items of dataset starting at 'begin'
// This does not check validity of the range!
template <typename Iter, typename Count>
constexpr view<Iter> view_n(Iter begin, Count n) {
        auto end = std::next(begin, n);
        return view<Iter>{std::move(begin), end};
}

// Creates the view over over Container, where we ignore first r*size/2 items
// and last r*size/2 items. This can be used to get the dataset without
// first/last 5% for example, by using r=0.1
template <typename Container>
constexpr view<iterator_of_t<Container>> trim_view(Container &cont, float r) {
        std::size_t step = cont.size() * (1 - r) / 2.f;
        return {cont.begin() + step, cont.end() - step};
}

// Returns view to the Container in reverse order.
template <typename Container,
          typename = std::enable_if_t<std::is_reference_v<Container> || is_view_v<Container>>>
constexpr auto reversed(Container &&container) -> view<decltype(container.rbegin())> {
        return {container.rbegin(), container.rend()};
}

} // namespace emlabcpp

namespace emlabcpp {

using std::abs;
using std::max;
using std::min;

constexpr float DEFAULT_EPSILON = 1.19e-07f;

// Sometimes necessary to disable warnings of unused arguments
template <typename T>
constexpr void ignore(T &&) {}

// Callable object that is inspired by std::identity
struct identity {
        template <typename T>
        [[nodiscard]] constexpr T &&operator()(T &&t) const noexcept {
                return std::forward<T>(t);
        }
};

// returns sign of variable T: -1,0,1
template <typename T>
constexpr int sign(T &&val) {
        if (T{0} > val) {
                return -1;
        }
        if (T{0} < val) {
                return 1;
        }
        return 0;
}

template <typename T>
constexpr T clamp(T val, T from, T to) {
        if (val < from) {
                return from;
        }
        if (val > to) {
                return to;
        }
        return val;
}

// maps input value 'input' from input range to equivalent value in output range
template <typename U, typename T>
constexpr U map_range(T input, T from_min, T from_max, U to_min, U to_max) {
        return to_min + (to_max - to_min) * (input - from_min) / (from_max - from_min);
}

// Returns the size of the container, regardless of what it is
template <typename Container>
[[nodiscard]] constexpr std::size_t cont_size(const Container &cont) noexcept {
        if constexpr (has_static_size_v<Container>) {
                return static_size_v<Container>;
        } else {
                return cont.size();
        }
}

// two items 'lh' and 'rh' are almost equal if their difference is smaller than
// value 'eps'
template <typename T>
[[nodiscard]] constexpr bool almost_equal(const T &lh, const T &rh, float eps = DEFAULT_EPSILON) {
        return float(abs(lh - rh)) < eps;
}

// Takes Callable object 'f', which takes multiple arguments and returns
// function that calls 'f', but accepts tuple of arguments, rather than the
// arguments directly
//
// Note: returned function has copy of callable object 'f'
template <typename Callable>
[[nodiscard]] constexpr auto curry(Callable &&f) {
        return [ff = std::forward<Callable>(f)](auto &&i_tuple) { //
                return std::apply(ff, std::forward<decltype(i_tuple)>(i_tuple));
        };
}

// Returns range over Container, which skips first item of container
template <typename Container, typename Iterator = iterator_of_t<Container>>
[[nodiscard]] constexpr view<Iterator> tail(Container &&cont, int step = 1) {
        static_assert(!std::is_rvalue_reference_v<Container> || is_view_v<Container>,
                      "tail() does not accept rvalue references except for types which "
                      "can be considered a view.");
        using namespace std;
        return view<Iterator>(begin(cont) + step, end(cont));
}

// Returns range over Container, which skips last item of container
template <typename Container, typename Iterator = iterator_of_t<Container>>
[[nodiscard]] constexpr view<Iterator> init(Container &&cont, int step = 1) {
        static_assert(!std::is_rvalue_reference_v<Container> || is_view_v<Container>,
                      "init() does not accept rvalue references except for types which "
                      "can be considered a view.");
        using namespace std;
        return view<Iterator>(begin(cont), end(cont) - step);
}

// Returns iterator for first item, for which call to f(*iter) holds true. end()
// iterator is returned otherwise. The end() iterator is taken once, before the
// container is iterated.
template <typename Container, typename UnaryFunction = identity,
          typename = std::enable_if_t<!is_std_tuple_v<Container>>>
[[nodiscard]] constexpr iterator_of_t<Container> find_if(Container &&    cont,
                                                         UnaryFunction &&f = identity()) {
        static_assert(!std::is_rvalue_reference_v<Container> || is_view_v<Container>,
                      "find_if() does not accept rvalue references except for types "
                      "which can be considered a view.");
        using namespace std;
        auto beg = begin(cont);
        auto end = cont.end();
        for (; beg != end; ++beg) {
                if (f(*beg)) {
                        return beg;
                }
        }
        return cont.end();
}

// Returns index of an element in tuple 't', for which call to f(x) holds true,
// otherwise returns index of 'past the end' item - size of the tuple
template <typename... Args, typename UnaryFunction = identity>
[[nodiscard]] constexpr std::size_t find_if(const std::tuple<Args...> &t,
                                            UnaryFunction &&           f = identity()) {
        return detail::find_if_impl(t, std::forward<UnaryFunction>(f),
                                    std::index_sequence_for<Args...>{});
}

// Finds first item in container 'cont' that is equal to 'item', returns
// iterator for container and index for tuples
template <typename Container, typename T>
[[nodiscard]] constexpr auto find(Container &&cont, const T &item) {
        static_assert(!std::is_rvalue_reference_v<Container> || is_view_v<Container>,
                      "find() does not accept rvalue references except for types which "
                      "can be considered a view.");
        return find_if(cont, [&](const auto &sub_item) { //
                return sub_item == item;
        });
}

// Applies unary function 'f' to each element of tuple 't'
template <typename Tuple, typename UnaryFunction>
constexpr std::enable_if_t<is_std_tuple_v<Tuple>, void> for_each(Tuple &&t, UnaryFunction &&f) {
        return detail::for_each_impl(std::forward<Tuple>(t), //
                                     std::forward<UnaryFunction>(f),
                                     std::make_index_sequence<cont_size(t)>{});
}

// Applies unary function 'f' to each element of container 'cont'
template <typename Container, typename UnaryFunction>
constexpr std::enable_if_t<!is_std_tuple_v<Container>, void> for_each(Container &&    cont,
                                                                      UnaryFunction &&f) {
        for (auto &&item : std::forward<Container>(cont)) {
                f(std::forward<decltype(item)>(item));
        }
}

// Helper structure for finding the smallest and the largest item in some
// container, contains min/max attributes representing such elements.
template <typename T>
struct min_max {
        T min{};
        T max{};
};

// Applies unary function 'f(x)' to each element of container 'cont', returns
// the largest and the smallest return value. of 'f(x)' calls. Returns the
// default value of the 'f(x)' return type if container is empty.
template <typename Container, typename UnaryFunction = identity,
          typename T = std::decay_t<mapped_t<Container, UnaryFunction>>>
[[nodiscard]] constexpr min_max<T> min_max_elem(const Container &cont,
                                                UnaryFunction && f = identity()) {
        min_max<T> res;
        res.max = std::numeric_limits<T>::lowest();
        res.min = std::numeric_limits<T>::max();

        for_each(cont, [&](const auto &item) {
                auto val = f(item);
                res.max  = max(res.max, val);
                res.min  = min(res.min, val);
        });
        return res;
}

// Applies unary function 'f(x)' to each element of container 'cont', returns
// the largest return value of 'f(x)' calls. Returns lowest value of the return
// type if container is empty.
template <typename Container, typename UnaryFunction = identity,
          typename T = std::decay_t<mapped_t<Container, UnaryFunction>>>
[[nodiscard]] constexpr T max_elem(const Container &cont, UnaryFunction &&f = identity()) {
        auto val = std::numeric_limits<T>::lowest();
        for_each(cont, [&](const auto &item) { //
                val = max(f(item), val);
        });
        return val;
}

// Applies unary function 'f(x) to each element of container 'cont, returns the
// smallest return value of 'f(x)' calls. Returns maximum value of the return
// type if container is empty.
template <typename Container, typename UnaryFunction = identity,
          typename T = std::remove_reference_t<mapped_t<Container, UnaryFunction>>>
[[nodiscard]] constexpr T min_elem(const Container &cont, UnaryFunction &&f = identity()) {
        auto val = std::numeric_limits<T>::max();
        for_each(cont, [&](const auto &item) { //
                val = min(f(item), val);
        });
        return val;
}

// Applies the unary function 'f(x)' to each element of container 'cont' and
// returns the count of items, for which f(x) returned 'true'
template <typename Container, typename UnaryFunction = identity>
[[nodiscard]] constexpr std::size_t count(const Container &cont, UnaryFunction &&f = identity()) {
        std::size_t res = 0;
        for_each(cont, [&](const auto &item) {
                if (f(item)) {
                        res += 1;
                }
        });
        return res;
}

// Applies f(x) to each item of container 'cont', returns the sum of all the
// return values of each call to 'f(x)' and 'init' item
template <typename Container, typename UnaryFunction = identity,
          typename T = std::decay_t<mapped_t<Container, UnaryFunction>>>
[[nodiscard]] constexpr T sum(const Container &cont, UnaryFunction &&f = identity(), T init = {}) {
        for_each(cont, [&](const auto &item) { //
                init += f(item);
        });
        return init;
}

// Applies function 'f(init,x)' to each element of container 'x' and actual
// value of 'init' in iteration, the return value is 'init' value for next round
template <typename Container, typename T, typename BinaryFunction>
[[nodiscard]] constexpr T accumulate(const Container &cont, T init, BinaryFunction &&f) {
        for_each(cont, [&](const auto &item) { //
                init = f(std::move(init), item);
        });
        return init;
}

// Applies function 'f(x)' to each element of container 'cont' and returns the
// average value of each call
template <typename Container, typename UnaryFunction = identity,
          typename T = std::decay_t<mapped_t<Container, UnaryFunction>>>
[[nodiscard]] constexpr T avg(const Container &cont, UnaryFunction &&f = identity()) {
        T res{};
        for_each(cont, [&](const auto &item) { //
                res += f(item);
        });
        return res / cont_size(cont);
}

// Applies binary function 'f(x,y)' to each combination of items x in lh_cont
// and y in rh_cont
template <typename LhContainer, typename RhContainer, typename BinaryFunction>
constexpr void for_cross_joint(LhContainer &&lh_cont, RhContainer &&rh_cont, BinaryFunction &&f) {
        for_each(lh_cont, [&](auto &lh_item) {         //
                for_each(rh_cont, [&](auto &rh_item) { //
                        f(lh_item, rh_item);
                });
        });
}

// Returns true if call to function 'f(x)' returns true for at least one item in
// 'cont'
template <typename Container, typename UnaryFunction>
[[nodiscard]] constexpr bool any_of(const Container &cont, UnaryFunction &&f) {
        auto res = find_if(cont, std::forward<UnaryFunction>(f));

        if constexpr (is_std_tuple_v<Container>) {
                return res != std::tuple_size_v<Container>;
        } else {
                return res != cont.end();
        }
}

// Returns true if call to function 'f(x)' returns false for all items in
// 'cont'.
template <typename Container, typename UnaryFunction>
[[nodiscard]] constexpr bool none_of(const Container &cont, UnaryFunction &&f) {
        return !any_of(cont, std::forward<UnaryFunction>(f));
}

// Returns true if call to function 'f(x)' returns true for all items in 'cont'
template <typename Container, typename UnaryFunction>
[[nodiscard]] constexpr bool all_of(const Container &cont, UnaryFunction &&f) {
        return !any_of(cont, [&](const auto &item) { //
                return !f(item);
        });
}

// Returns true of containers 'lh' and 'rh' has same size and lh[i] == rh[i] for
// all 0 <= i < size()
template <typename LhContainer, typename RhContainer>
[[nodiscard]] constexpr bool equal(const LhContainer &lh, const RhContainer &rh) {
        if (lh.size() != rh.size()) {
                return false;
        }
        using namespace std;
        auto lbeg = begin(lh);
        auto rbeg = begin(rh);
        auto lend = end(lh);
        for (; lbeg != lend; ++lbeg, ++rbeg) {
                if (*lbeg != *rbeg) {
                        return false;
                }
        }
        return true;
}

// Calls function f(x) for each item in container 'cont' (or tuple) and stores
// result in 'ResultContainer', which is returned out of the function. The
// behavior depends on what kind of 'ResultContainer' is used, rules are in this
// order:
//  1. std::array is constructed and res[i] = f(cont[i]) is used for i = 0...N
//  2. if 'ResultContainer' has push_back(x) method, that is used to insert
//  result of calls to f(x)
//  3. insert(x) method is used to insert result of calls to f(x)
template <typename ResultContainer, typename Container, typename UnaryFunction = identity>
[[nodiscard]] inline ResultContainer map_f(Container &&cont, UnaryFunction &&f = identity()) {
        static_assert(!is_std_tuple_v<ResultContainer>,
                      "This version of map_f does not work with std::tuple as "
                      "_result_ container!");

        ResultContainer res{};

        std::size_t i = 0;
        for_each(std::forward<Container>(cont), [&](auto &&item) {
                using T = decltype(item);
                if constexpr (is_std_array_v<ResultContainer>) {
                        res[i] = f(std::forward<T>(cont[i]));
                        ++i;
                } else if constexpr (has_push_back_v<ResultContainer>) {
                        res.push_back(f(std::forward<T>(item)));
                } else {
                        res.insert(f(std::forward<T>(item)));
                }
        });
        return res;
}

// Calls function f(cont[i]) for i = 0...N and stores the result in array of an
// appropiate size. The functions needs size 'N' as template parameter
template <std::size_t N, typename Container, typename UnaryFunction = identity,
          typename T = mapped_t<Container, UnaryFunction>,
          typename   = std::enable_if_t<!has_static_size_v<Container>>>
[[nodiscard]] inline std::array<T, N> map_f_to_a(Container &&cont, UnaryFunction &&f = identity()) {
        return detail::map_f_to_a_impl<T, N>(std::forward<Container>(cont),
                                             std::forward<UnaryFunction>(f));
}

// Calls function f(cont[i]) for i = 0...N and stores the result in array of an
// appropiate size.
template <typename Container, std::size_t N = static_size_v<Container>,
          typename UnaryFunction = identity, typename T = mapped_t<Container, UnaryFunction>,
          typename = std::enable_if_t<has_static_size_v<Container>>>
[[nodiscard]] inline std::array<T, N> map_f_to_a(Container &&cont, UnaryFunction &&f = identity()) {
        return detail::map_f_to_a_impl<T, N>(std::forward<Container>(cont),
                                             std::forward<UnaryFunction>(f));
}

// Returns cont[0] + val + cont[1] + val + cont[2] + ... + cont[n-1] + val +
// cont[n];
template <typename Container, typename T>
[[nodiscard]] constexpr T joined(const Container &cont, T &&val) {
        if (cont.empty()) {
                return T{};
        }
        using namespace std;
        T res = *begin(cont);
        for (const auto &item : tail(cont)) {
                res += val + item;
        }
        return res;
}

} // namespace emlabcpp

#include <cstdint>
#include <limits>
#include <new>
#include <type_traits>
#include <utility>

namespace emlabcpp {

// Class implementing circular buffer of any type for up to N elements. This should work for generic
// type T, not just simple types.
//
// It is safe in "single consumer single producer" scenario between main loop and interrupts.
// Because of that the behavior is as follows:
//  - on insertion, item is inserted and than index is advanced
//  - on removal, item is removed and than index is advanced
//
// In case of copy or move operations, the buffer does not have to store the data internally in same
// manner, the data are equivavlent only from the perspective of push/pop operations.
//
template <typename T, std::size_t N>
class static_circular_buffer {
        // We need real_size of the buffer to be +1 bigger than number of items
        static constexpr std::size_t real_size = N + 1;
        // type for storage of one item
        using storage_type = std::aligned_storage_t<sizeof(T), alignof(T)>;

      public:
        // public types
        // --------------------------------------------------------------------------------
        using value_type      = T;
        using size_type       = std::size_t;
        using reference       = T &;
        using const_reference = const T &;

        // public methods
        // --------------------------------------------------------------------------------
        static_circular_buffer() = default;
        static_circular_buffer(const static_circular_buffer &other) {
                for (size_type i = 0; i < other.size(); ++i) {
                        push_back(other[i]);
                }
        }
        static_circular_buffer(static_circular_buffer &&other) {
                while (!other.empty()) {
                        push_back(other.pop_front());
                }
        }
        static_circular_buffer &operator=(const static_circular_buffer &other) {
                this->~static_circular_buffer();
                ::new (this) static_circular_buffer(other);
                return *this;
        }
        static_circular_buffer &operator=(static_circular_buffer &&other) {
                this->~static_circular_buffer();
                ::new (this) static_circular_buffer(std::move(other));
                return *this;
        }

        // methods for handling the front side of the circular buffer

        [[nodiscard]] reference       front() { return ref_item(from_); }
        [[nodiscard]] const_reference front() const { return ref_item(from_); }

        T pop_front() {
                T item = std::move(front());
                delete_item(from_);
                from_ = next(from_);
                return item;
        }

        // methods for handling the back side of the circular buffer

        [[nodiscard]] reference       back() { return ref_item(to_ - 1); }
        [[nodiscard]] const_reference back() const { return ref_item(to_ - 1); }

        void push_back(T item) { emplace_back(std::move(item)); }

        template <typename... Args>
        void emplace_back(Args &&... args) {
                emplace_item(to_, std::forward<Args>(args)...);
                to_ = next(to_);
        }

        // other methods

        [[nodiscard]] constexpr std::size_t max_size() const { return N; }

        [[nodiscard]] std::size_t size() const {
                if (to_ >= from_) {
                        return to_ - from_;
                }
                return to_ + (real_size - from_);
        }

        [[nodiscard]] bool empty() const { return to_ == from_; }

        [[nodiscard]] bool full() const { return next(to_) == from_; }

        const_reference operator[](size_type i) const { return ref_item((from_ + i) % real_size); }
        reference       operator[](size_type i) { return ref_item((from_ + i) % real_size); }

        void clear() { purge(); }

        ~static_circular_buffer() { purge(); }

      private:
        // private attributes
        // --------------------------------------------------------------------------------

        storage_type data_[real_size]; // storage of the entire dataset
        size_type    from_ = 0;        // index of the first item
        size_type    to_   = 0;        // index past the last item

        // from_ == to_ means empty
        // to_ + 1 == from_ is full

        // private methods
        // --------------------------------------------------------------------------------
        // To understand std::launder:
        // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0532r0.pdf
        //
        // Set of [delete,init,emplace]_item methods is necessary as data_ is not array of T, but
        // array of byte-like-type that can store T -> T does not have to be initialized there. We
        // want to fully support T objects - their constructors/destructors are correctly called and
        // we do not require default constructor. This implies that data_ has some slots
        // un-initialized, some are initialized and we have to handle them correctly.
        //
        // All three methods are used to handle this part of the objects in this scenario, that
        // requires features of C++ we do not want to replicate and it's bettter to hide them in
        // methods.

        void delete_item(size_type i) { ref_item(i).~T(); }

        template <typename... Args>
        void emplace_item(size_type i, Args &&... args) {
                void *gen_ptr = reinterpret_cast<void *>(&data_[i]);
                ::new (gen_ptr) T(std::forward<Args>(args)...);
        }

        // Reference to the item in data_storage. std::launder is necessary here per the paper
        // linked above.
        reference ref_item(size_type i) { return *std::launder(reinterpret_cast<T *>(&data_[i])); }
        const_reference ref_item(size_type i) const {
                return *std::launder(reinterpret_cast<const T *>(&data_[i]));
        }

        // Cleans entire buffer from items.
        void purge() {
                while (!empty()) {
                        pop_front();
                }
        }

        // Use this only when moving the indexes in the circular buffer - bullet-proof.
        constexpr auto next(size_type i) const { return (i + 1) % real_size; }
        constexpr auto prev(size_type i) const { return i == 0 ? real_size - 1 : i - 1; }
};

template <typename T, std::size_t N>
[[nodiscard]] inline bool operator==(const static_circular_buffer<T, N> &lh,
                                     const static_circular_buffer<T, N> &rh) {
        auto size = lh.size();
        if (size != rh.size()) {
                return false;
        }

        for (std::size_t i = 0; i < size; ++i) {
                if (lh[i] != rh[i]) {
                        return false;
                }
        }
        return true;
}

template <typename T, std::size_t N>
[[nodiscard]] inline bool operator!=(const static_circular_buffer<T, N> &lh,
                                     const static_circular_buffer<T, N> &rh) {
        return !(lh == rh);
}

} // namespace emlabcpp

#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

namespace emlabcpp {

// Either is heterogenous structure that holds one of the two types specified.
// This is stored as union, so the memory requirement of either is always the size of the bigger
// element + constant for identifying which on is contained.
//
// The intervall value can't be directly accessed, unless both sides are same, in which case join()
// returns the internal value. (Which is safe if both are same) In case they are different, you can
// only access the value by either convert_left/right, bind_left/right or match, which calls
// appropiate callable based on whenever left or right item is present.
//
// This prevents user of the code from errors in accessing proper side of the either.
template <typename LH, typename RH>
class either {
        union {
                LH left_;
                RH right_;
        };

        enum class item : uint8_t { LEFT = 0, RIGHT = 1 };

        item id_;

      public:
        using left_item  = LH;
        using right_item = RH;

        either(left_item &&item) noexcept : id_(item::LEFT) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                new (&left_) left_item(std::move(item));
        }

        either(const left_item &item) noexcept : id_(item::LEFT) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                new (&left_) left_item(item);
        }

        template <typename U = RH, typename = std::enable_if_t<!std::is_same_v<LH, U>>>
        either(RH &&item) noexcept : id_(item::RIGHT) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                new (&right_) right_item(std::move(item));
        }

        template <typename U = RH, typename = std::enable_if_t<!std::is_same_v<LH, U>>>
        either(const RH &item) noexcept : id_(item::RIGHT) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                new (&right_) right_item(item);
        }

        either(const either &other) noexcept : id_(other.id_) {
                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        new (&left_) left_item(other.left_);
                } else {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        new (&right_) right_item(other.right_);
                }
        }

        either(either &&other) noexcept : id_(other.id_) {
                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        new (&left_) left_item(std::move(other.left_));
                } else {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        new (&right_) right_item(std::move(other.right_));
                }
        }

        either &operator=(const either &other) {
                if (other.id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        *this = other.left_;
                } else {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        *this = other.right_;
                }
                return *this;
        }

        either &operator=(either &&other) noexcept {
                if (other.id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        *this = std::move(other.left_);
                } else {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        *this = std::move(other.right_);
                }
                return *this;
        }

        either &operator=(const left_item &other) {
                destruct();
                id_ = item::LEFT;
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                new (&left_) left_item(other);
                return *this;
        }
        either &operator=(left_item &&other) {
                destruct();
                id_ = item::LEFT;
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                new (&left_) left_item(std::move(other));
                return *this;
        }

        template <typename U = right_item, typename = std::enable_if_t<!std::is_same_v<LH, U>>>
        either &operator=(const right_item &other) {
                destruct();
                id_ = item::RIGHT;
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                new (&right_) right_item(other);
                return *this;
        }

        template <typename U = right_item, typename = std::enable_if_t<!std::is_same_v<LH, U>>>
        either &operator=(RH &&other) {
                destruct();
                id_ = item::RIGHT;
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                new (&right_) right_item(std::move(other));
                return *this;
        }

        constexpr bool is_left() const { return id_ == item::LEFT; }

        template <typename LeftFunction>
        auto convert_left(LeftFunction &&left_f) & {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                using return_either = either<decltype(left_f(left_)), right_item>;

                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        return return_either{left_f(left_)};
                }

                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                return return_either{right_};
        }

        template <typename LeftFunction>
        auto convert_left(LeftFunction &&left_f) && {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                using return_either = either<decltype(left_f(std::move(left_))), right_item>;

                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        return return_either{left_f(std::move(left_))};
                }

                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                return return_either{std::move(right_)};
        }

        template <typename RightFunction>
        auto convert_right(RightFunction &&right_f) & {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                using return_either = either<left_item, decltype(right_f(right_))>;

                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        return return_either{left_};
                }

                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                return return_either{right_f(right_)};
        }

        template <typename RightFunction>
        auto convert_right(RightFunction &&right_f) && {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                using return_either = either<left_item, decltype(right_f(std::move(right_)))>;

                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        return return_either{std::move(left_)};
                }

                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                return return_either{right_f(std::move(right_))};
        }

        template <typename LeftFunction, typename RightFunction>
        void match(LeftFunction &&left_f, RightFunction &&right_f) & {
                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        left_f(left_);
                } else {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        right_f(right_);
                }
        }

        template <typename LeftFunction, typename RightFunction>
        void match(LeftFunction &&left_f, RightFunction &&right_f) const & {
                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        left_f(left_);
                } else {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        right_f(right_);
                }
        }

        template <typename LeftFunction, typename RightFunction>
        void match(LeftFunction &&left_f, RightFunction &&right_f) && {
                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        left_f(std::move(left_));
                } else {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        right_f(std::move(right_));
                }
        }

        template <typename U = left_item, typename K = right_item>
        std::enable_if_t<std::is_same_v<U, K>, left_item> join() && {
                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        return std::move(left_);
                }

                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                return std::move(right_);
        }
        template <typename U = left_item, typename K = right_item>
        std::enable_if_t<std::is_same_v<U, K>, LH> join() & {
                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        return left_;
                }

                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                return right_;
        }

        template <typename UnaryFunction>
        auto bind_left(UnaryFunction &&left_f) & {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                using return_either = decltype(left_f(left_));

                static_assert(std::is_same_v<typename return_either::right_item, right_item>,
                              "In bind left, the right_types has to be same!");

                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        return left_f(left_);
                }

                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                return return_either{right_};
        }
        template <typename UnaryFunction>
        auto bind_left(UnaryFunction &&left_f) && {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                using return_either = decltype(left_f(std::move(left_)));

                static_assert(std::is_same_v<typename return_either::right_item, right_item>,
                              "In bind left, the right_types has to be same!");

                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        return left_f(std::move(left_));
                }

                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                return return_either{std::move(right_)};
        }

        ~either() { destruct(); }

      private:
        void destruct() {
                if (id_ == item::LEFT) {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        left_.~left_item();
                } else {
                        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                        right_.~right_item();
                }
        }
};

// Marks empty assembly
struct empty_assembly_tag {};

// Function gets a set of various std::optionals and either returns all their values assembled as
// tuple or 'empty_assembly_tag' implicating that some of the optionals was empty.
template <typename... Ts>
inline either<std::tuple<Ts...>, empty_assembly_tag>
assemble_optionals(std::optional<Ts> &&... opt) {
        if ((... && opt)) {
                return std::make_tuple<Ts...>(std::forward<Ts>(*opt)...);
        }

        return empty_assembly_tag{};
}

// Function expects eithers of any left type, but same right type.
// These are acceessed and either tuple of _all_ left items is returned or vector of any of right
// items.
//
// This returns appropiated either< std::tuple< LeftItems... >, static_circular_buffer< Righitems, N
// >>. This is handy for uses cases when you have expected values of multiple functions on the left,
// and their errors on the right. It either returns all values or the errors that happend.

template <typename FirstE, typename... Eithers>
inline auto assemble_left_collect_right(FirstE &&first, Eithers &&... others) {
        static_assert(are_same_v<typename Eithers::right_item...>,
                      "Right items of Eithers have to be same for collection!");

        using right_type        = typename FirstE::right_item;
        constexpr std::size_t N = 1 + sizeof...(Eithers);

        static_circular_buffer<right_type, N> collection;

        auto convert = [&](auto &&either) {
                using eitherT   = decltype(either);
                using left_type = typename std::remove_reference_t<eitherT>::left_item;

                return std::forward<eitherT>(either)
                    .convert_left([&](auto &&item) { //
                            return std::make_optional(std::forward<decltype(item)>(item));
                    })
                    .convert_right([&](auto &&item) {
                            collection.push_back(std::forward<decltype(item)>(item));
                            return std::optional<left_type>();
                    })
                    .join();
        };

        return assemble_optionals(convert(std::forward<FirstE>(first)),
                                  convert(std::forward<Eithers>(others))...)
            .convert_right([&](empty_assembly_tag) { //
                    return collection;
            });
}
} // namespace emlabcpp

#include <iterator>

namespace emlabcpp {

template <typename T>
struct generic_iterator_traits;

template <typename T>
class generic_iterator {

        T &      impl() { return static_cast<T &>(*this); }
        T const &impl() const { return static_cast<T const &>(*this); }

      public:
        using value_type        = typename generic_iterator_traits<T>::value_type;
        using reference         = typename generic_iterator_traits<T>::reference;
        using const_reference   = typename generic_iterator_traits<T>::const_reference;
        using pointer           = typename generic_iterator_traits<T>::pointer;
        using const_pointer     = typename generic_iterator_traits<T>::const_pointer;
        using difference_type   = typename generic_iterator_traits<T>::difference_type;
        using iterator_category = std::bidirectional_iterator_tag;

        // Dereference to the internal driver value
        constexpr pointer operator->() { return &*impl(); }

        // Const dereference to the internal driver value
        constexpr const_pointer operator->() const { return &*impl(); }

        // Advances the driver by '1' using Driver::next(1)
        constexpr T &operator++() {
                impl() += 1;
                return impl();
        }

        // Advances the driver by '1' using Driver::next(1)
        constexpr T &operator++(int) {
                auto copy = impl();
                impl() += 1;
                return copy;
        }

        // Steps back the driver by '1' using Driver::retreat(1)
        constexpr T &operator--() {
                impl() -= 1;
                return impl();
        }

        // Steps back the driver by '1' using Driver::retreat(1)
        constexpr T &operator--(int) {
                auto copy = impl();
                impl() -= 1;
                return copy;
        }

        // Compares the iterators using Driver::less_than.
        constexpr bool operator<(const generic_iterator<T> &other) const {
                return impl() < other.impl();
        }

        // Compares the iterators using Driver::equals
        constexpr bool operator==(const generic_iterator<T> &other) const {
                return impl() == other.impl();
        }
};

// A > B iff B < A
template <typename T>
constexpr bool operator>(const generic_iterator<T> &lh, const generic_iterator<T> &rh) {
        return rh < lh;
}

// A <= B iff !( B > A )
template <typename T>
constexpr bool operator<=(const generic_iterator<T> &lh, const generic_iterator<T> &rh) {
        return !(lh > rh);
}

// A >= B iff !( B < A )
template <typename T>
constexpr bool operator>=(const generic_iterator<T> &lh, const generic_iterator<T> &rh) {
        return !(lh < rh);
}

// A != B iff !( A == B)
template <typename T>
constexpr bool operator!=(const generic_iterator<T> &lh, const generic_iterator<T> &rh) {
        return !(lh == rh);
}

} // namespace emlabcpp

#include <cmath>
#include <functional>
#include <limits>
#include <ratio>
#include <type_traits>

namespace emlabcpp {

/** Class representing generic quantity.
 *
 * Quantities are types which simply overlay basic numeric type (ValueType) and are tagged with some
 * unique type (Tag). The C++ type system prevents you from passing values of quantites of different
 * tags, unless explicitly stated!
 *
 * So if your function expects quantity with tag 'distance_tag', you can't pass it 'velocity_tag'.
 *
 * Only quantities of same Tag and ValueType are allowed following operations:
 * 	+=,-=
 * 	+,-
 * 	==, !=
 * 	<,>,>=,<=
 * 	abs, max, min
 * Quantity can be multiplied or divided by it's ValueType - /,*,/=,*=
 * Additionally, we support these operations over quantity:
 * 	cos, sin
 *
 * @param Tag template param that specifies the semantical meaning of the physical quantity.
 *
 * Credits should go to https://github.com/joboccara/NamedType as I inspired by project by this
 * blogger!
 */
template <typename T, typename ValueType = float>
class quantity {
        ValueType value_;

        T &      impl() { return static_cast<T &>(*this); }
        T const &impl() const { return static_cast<T const &>(*this); }

      public:
        using value_type = ValueType;

        constexpr quantity() noexcept : value_(0) {}

        // Default constructor used to create a physical quantity from value
        constexpr explicit quantity(ValueType val) noexcept : value_(val) {}

        // Const reference to the internal value
        constexpr ValueType operator*() const noexcept { return value_; }

        // Add other quantity of same tag and value_type
        constexpr T &operator+=(const quantity other) noexcept {
                value_ += *other;
                return impl();
        }

        // Subtract other quantity of same tag and value_type
        constexpr T &operator-=(const quantity other) noexcept {
                value_ -= *other;
                return impl();
        }

        // Divides quantity by it's value type
        constexpr T &operator/=(const ValueType val) noexcept {
                value_ /= val;
                return impl();
        }

        // Multiplies quantity by it's value type
        constexpr T &operator*=(const ValueType val) noexcept {
                value_ *= val;
                return impl();
        }

        // Provides explicit conversion of internal value to type U
        template <typename U>
        constexpr explicit operator U() const noexcept {
                return U(value_);
        }
};

// Sum of quantities with same tag and value_type
template <typename T, typename ValueType>
constexpr T operator+(quantity<T, ValueType> lhs, const quantity<T, ValueType> rhs) {
        return lhs += rhs;
}

// Subtraction of quantities with same tag and value_type
template <typename T, typename ValueType>
constexpr T operator-(quantity<T, ValueType> lhs, const quantity<T, ValueType> rhs) {
        return lhs -= rhs;
}

// Provides negation of the quantity
template <typename T, typename ValueType>
constexpr T operator-(const quantity<T, ValueType> val) {
        return T{-*val};
}

// Comparison of internal values of quantity
template <typename T, typename ValueType>
constexpr bool operator==(const quantity<T, ValueType> lhs, const quantity<T, ValueType> rhs) {
        return *lhs == *rhs;
}

// Comparasion of internal values
template <typename T, typename ValueType>
constexpr bool operator<(const quantity<T, ValueType> lhs, const quantity<T, ValueType> rhs) {
        return *lhs < *rhs;
}

// Multiplication of quantity by it's value_type
template <typename T, typename ValueType>
constexpr T operator*(quantity<T, ValueType> q, const ValueType val) {
        return q *= val;
}

// Division of quantity by it's value_type
template <typename T, typename ValueType>
constexpr T operator/(quantity<T, ValueType> q, const ValueType val) {
        return q /= val;
}

// Quantity with absolute value of internal value
template <typename T, typename ValueType>
constexpr T abs(const quantity<T, ValueType> q) {
        return T(std::abs(*q));
}

// Returns cosinus of the quantity - untagged
template <typename T, typename ValueType>
constexpr double cos(const quantity<T, ValueType> u) {
        return std::cos(*u);
}

// Returns sinus of the quantity - untagged
template <typename T, typename ValueType>
constexpr double sin(const quantity<T, ValueType> u) {
        return std::sin(*u);
}

// Quantity with maximum value of one of the quantities
template <typename T, typename ValueType>
constexpr T max(const quantity<T, ValueType> lh, const quantity<T, ValueType> rh) {
        return T(std::max(*lh, *rh));
}

// Quantity with minimum value of one of the quantities
template <typename T, typename ValueType>
constexpr T min(const quantity<T, ValueType> lh, const quantity<T, ValueType> rh) {
        return T(std::min(*lh, *rh));
}

//---------------------------------------------------------------------------

// Non-equality of quantites is negation of equality.
template <typename T, typename ValueType>
constexpr bool operator!=(const quantity<T, ValueType> lhs, const quantity<T, ValueType> rhs) {
        return !(lhs == rhs);
}

// Q1 > Q2 iff Q2 < Q1
template <typename T, typename ValueType>
constexpr bool operator>(const quantity<T, ValueType> lhs, const quantity<T, ValueType> rhs) {
        return rhs < lhs;
}
// Q1 <= Q2 iff !( Q2 > Q1 )
template <typename T, typename ValueType>
constexpr bool operator<=(const quantity<T, ValueType> lhs, const quantity<T, ValueType> rhs) {
        return !(lhs > rhs);
}
// Q1 >= Q2 iff !( Q2 < Q1 )
template <typename T, typename ValueType>
constexpr bool operator>=(const quantity<T, ValueType> lhs, const quantity<T, ValueType> rhs) {
        return !(lhs < rhs);
}
//---------------------------------------------------------------------------

// Multiplication of value_type by quantity returns quantity
template <typename T, typename ValueType>
constexpr T operator*(const ValueType val, const quantity<T, ValueType> q) {
        return q * val;
}
// Division of value_type by quantity returns quantity
template <typename T, typename ValueType>
constexpr ValueType operator/(const ValueType val, const quantity<T, ValueType> q) {
        return val / *q;
}

} // namespace emlabcpp

// The quantity has defined partital specialization of std::numeric_limits,
// works as is intuitive.
template <typename T, typename ValueType>
class std::numeric_limits<emlabcpp::quantity<T, ValueType>> {
      public:
        constexpr static emlabcpp::quantity<T, ValueType> lowest() {
                return emlabcpp::quantity<T, ValueType>{std::numeric_limits<ValueType>::lowest()};
        }
        constexpr static emlabcpp::quantity<T, ValueType> min() {
                return emlabcpp::quantity<T, ValueType>{std::numeric_limits<ValueType>::min()};
        }
        constexpr static emlabcpp::quantity<T, ValueType> max() {
                return emlabcpp::quantity<T, ValueType>{std::numeric_limits<ValueType>::max()};
        }
};

// Hash of quantity is hash of it's value and Tag::get_unit() xored.
template <typename T, typename ValueType>
struct std::hash<emlabcpp::quantity<T, ValueType>> {
        std::size_t operator()(const emlabcpp::quantity<T, ValueType> q) {
                // TODO: this should be rewritten
                // 'reverse' the prefix+unit info in bits and than xor it with number
                std::string unit = T::get_unit();
                return std::hash<ValueType>()(*q) ^ std::hash<std::string>()(unit);
        }
};

namespace emlabcpp {

// Physical quantity tag represents all physical units defined using the International System of
// Units and more. The idea is that each used unit, is either one of the seven basic units, or
// defined as some combination of them.  The combination is multiplication of exponents of basic
// units.
//
// So, velocity is distance per time - m*s^-1.
//
// We defined each physical unit by defining a tag, that has exponent of each basic unit as
// exponent. This makes it possible to specify any unit using this tag.
//
// We expand this by providing two additional 'basic' units - angle (which is handy for us) and
// byte.
//
// Given that we have the exponents of basic units as integers in the type, we can write generic
// multiplication and division between all possible tags.
//
// This trick is inspired by the haskell's dimensional library which does the same.
//
template <int l, int mass, int t, int current, int temp, int mol, int li, int angle, int byte>
struct physical_quantity
    : quantity<physical_quantity<l, mass, t, current, temp, mol, li, angle, byte>, float> {

        using quantity<physical_quantity<l, mass, t, current, temp, mol, li, angle, byte>,
                       float>::quantity;

        static std::string get_unit() {
                auto seg = [](std::string unit, int i) -> std::string {
                        if (i == 0) {
                                return std::string();
                        }
                        if (i == 1) {
                                return unit;
                        }
                        return unit + "^" + std::to_string(i);
                };
                return seg("m", l) + seg("g", mass) + seg("s", t) + seg("A", current) +
                       seg("K", temp) + seg("mol", mol) + seg("cd", li) + seg("rad", angle) +
                       seg("B", byte);
        }
};

// Table of alieses for most used physical units
using unitless            = physical_quantity<0, 0, 0, 0, 0, 0, 0, 0, 0>;
using length              = physical_quantity<1, 0, 0, 0, 0, 0, 0, 0, 0>;
using mass                = physical_quantity<0, 1, 0, 0, 0, 0, 0, 0, 0>;
using timeq               = physical_quantity<0, 0, 1, 0, 0, 0, 0, 0, 0>;
using current             = physical_quantity<0, 0, 0, 1, 0, 0, 0, 0, 0>;
using temp                = physical_quantity<0, 0, 0, 0, 1, 0, 0, 0, 0>;
using amount_of_substance = physical_quantity<0, 0, 0, 0, 0, 1, 0, 0, 0>;
using luminous_intensity  = physical_quantity<0, 0, 0, 0, 0, 0, 1, 0, 0>;
using angle               = physical_quantity<0, 0, 0, 0, 0, 0, 0, 1, 0>;
using byte                = physical_quantity<0, 0, 0, 0, 0, 0, 0, 0, 1>;
using angular_velocity    = physical_quantity<0, 0, -1, 0, 0, 0, 0, 1, 0>;
using area                = physical_quantity<2, 0, 0, 0, 0, 0, 0, 0, 0>;
using volume              = physical_quantity<3, 0, 0, 0, 0, 0, 0, 0, 0>;
using velocity            = physical_quantity<1, 0, -1, 0, 0, 0, 0, 0, 0>;
using frequency           = physical_quantity<0, 0, -1, 0, 0, 0, 0, 0, 0>;
using force               = physical_quantity<1, 1, -2, 0, 0, 0, 0, 0, 0>;
using power               = physical_quantity<2, 1, -3, 0, 0, 0, 0, 0, 0>;
using voltage             = physical_quantity<2, 1, -3, -1, 0, 0, 0, 0, 0>;
using resistance          = physical_quantity<2, 1, -3, -2, 0, 0, 0, 0, 0>;
using distance            = length;
using radius              = length;

// Constants of units that are relevant for us
constexpr angle PI = angle{float(std::acos(-1))};

// Multiplication of quantities of physical_quantiy_tag multiplies the internal
// values and the result is a type, where the exponents of each side of the
// multiplication are summed.
template <int l0, int mass0, int t0, int curr0, int temp0, int mol0, int li0, int angle0, int byte0,
          int l1, int mass1, int t1, int curr1, int temp1, int mol1, int li1, int angle1, int byte1,
          typename ValueType>
constexpr auto
operator*(physical_quantity<l0, mass0, t0, curr0, temp0, mol0, li0, angle0, byte0> lh,
          physical_quantity<l1, mass1, t1, curr1, temp1, mol1, li1, angle1, byte1> rh) {
        return physical_quantity<l0 + l1, mass0 + mass1, t0 + t1, curr0 + curr1, temp0 + temp1,
                                 mol0 + mol1, li0 + li1, angle0 + angle1, byte0 + byte1>{(*lh) *
                                                                                         (*rh)};
}

// Divison of quantities of physical_quantiy_tag divides the internal values and
// the result is a type, where the exponents of each side of the multiplication
// are subtracted.
template <int l0, int mass0, int t0, int curr0, int temp0, int mol0, int li0, int angle0, int byte0,
          int l1, int mass1, int t1, int curr1, int temp1, int mol1, int li1, int angle1, int byte1,
          typename ValueType>
constexpr auto
operator/(physical_quantity<l0, mass0, t0, curr0, temp0, mol0, li0, angle0, byte0> lh,
          physical_quantity<l1, mass1, t1, curr1, temp1, mol1, li1, angle1, byte1> rh) {
        return physical_quantity<l0 - l1, mass0 - mass1, t0 - t1, curr0 - curr1, temp0 - temp1,
                                 mol0 - mol1, li0 - li1, angle0 - angle1, byte0 - byte1>{(*lh) /
                                                                                         (*rh)};
}

// Square root of physical quantity is square root of it's value and the
// exponents are divided in half.
template <int l, int mass, int t, int curr, int temp, int mol, int li, int angle, int byte,
          typename ValueType>
constexpr auto sqrt(physical_quantity<l, mass, t, curr, temp, mol, li, angle, byte> val) {
        return physical_quantity<l / 2, mass / 2, t / 2, curr / 2, temp / 2, mol / 2, li / 2,
                                 angle / 2, byte / 2>{ValueType{std::sqrt(*val)}};
}

} // namespace emlabcpp

#inlude "view.h"
#include <tuple>

namespace emlabcpp {

/* zip_ierator iterates over a group of iterators, where value is a tuple of references to value for
 * each iterator.
 *
 * The design expects that all ranges of iterators are of same size.
 */
template <typename... Iterators>
class zip_iterator {
        std::tuple<Iterators...> iters_;

      public:
        constexpr zip_iterator(Iterators... iters) : iters_(std::move(iters)...) {}

        // Increases each iterator
        constexpr zip_iterator operator++() {
                std::apply(
                    [](auto &&... it) { //
                            (++it, ...);
                    },
                    iters_);

                return *this;
        }

        // Decreases each iterator
        constexpr zip_iterator operator--() {
                std::apply(
                    [](auto &&... it) { //
                            (++it, ...);
                    },
                    iters_);

                return *this;
        }

        // Dereference of each iterator, returns tuple of references to the
        // operator* of iterators.
        constexpr auto operator*() {
                return std::apply(
                    [](auto &&... it) { //
                            return std::forward_as_tuple((*it)...);
                    },
                    iters_);
        }

        // Two zip iterators are equal if all of their iterators are equal
        constexpr bool operator==(const zip_iterator<Iterators...> &other) const {
                return equals(other, std::index_sequence_for<Iterators...>{});
        }

      private:
        template <typename std::size_t... Idx>
        constexpr bool equals(const zip_iterator<Iterators...> &other,
                              std::index_sequence<Idx...>) const {
                return ((std::get<Idx>(iters_) == std::get<Idx>(other.iters_)) || ...);
        }
};

template <typename... Iterators>
constexpr bool operator!=(const zip_iterator<Iterators...> &lh,
                          const zip_iterator<Iterators...> &rh) {
        return !(lh == rh);
}

/* Creates a view of zip iterators for specified containers.
 *
 * Beware that the function does not check that containers have same size of
 * ranges. If the size differs, increments of begin iterator will never be same
 * as end iterator.
 */
template <typename... Ts>
inline auto zip(Ts &&... cont) {
        return view(zip_iterator(std::begin(cont)...), zip_iterator(std::end(cont)...));
}

} // namespace emlabcpp

#pragma once

// dummy file that serves as main header file including everything