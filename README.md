
<div align="center">

C++ embedded library

---

[Documentation](https://koniarik.github.io/emlabcpp/index.html)

</div>

An opinionated C++20 library focusing on embedded development.
It provides a wide set of tools, from complex mechanisms (protocol library) to simple utilites (view).

Want help? Find `veverak` on [Amulius Embedded Discord](https://discord.com/invite/embedded)

- [Installation](#Installation)
- [Components](#Components)
    - [algorithm.h](#algorithmh)
    - [assert.h](#asserth)
    - [bounded.h](#boundedh)
    - [concepts.h](#conceptsh)
    - [defer.h](#deferh)
    - [enum.h](#enumh)
    - [iterator.h](#iteratorh)
    - [match.h](#matchh)
    - [physical_quantity.h](#physical_quantityh)
    - [pid.h](#pidh)
    - [protocol.h](#protocolh)
    - [quantity.h](#quantityh)
    - [static_circular_buffer.h](#static_circular_bufferh)
    - [static_vector.h](#static_vectorh)
    - [types.h](#typesh)
    - [view.h](#viewh)
    - [visit.h](#visith)
    - [zip.h](#ziph)

## Installation
Repository is at https://github.com/koniarik/emlabcpp
The prefered of getting the library for now is via fetchcontent:

```cmake
FetchContent_Declare(
  emlabcpp
  GIT_REPOSITORY https://github.com/koniarik/emlabcpp
  GIT_TAG v1.1
)
FetchContent_MakeAvailable(emlabcpp)
```

## Components
The library can be view as a set of components.
These are organized based on the root header file for the said component.

### algorithm.h
Contains a set of algorithms similar to \<algorithm\> standard library, with a major change.
Functions take as an argument a container itself, rather than iterators.
Most of the functions are also able to work with `std::tuple`.
This is expanded with other short functions representing simple algorithms.

The two core functions are `find_if` and `for_each`, both are implemented with variant over containers and tuples.

```cpp
std::tuple< int, std::string > tpl_data;
std::vector< int > vec_data;
for_each(tpl_data, [&]( const auto & item ){
    std::cout << item << '\n';
});
for_each(vec_data, [&]( int item ){
    std::cout << item << '\n';
});

std::size_t index = find_if(tpl_data, [&]( auto item ){
    return std::is_same_v< decltype(item), std::string >;
});
auto iter = find_if(vec_data, [&]( int i ){
    return i != 0;
});

```

See examples for an overview of algorithms.

### assert.h

Provides `EMLABCPP_ASSERT(c)` macro that has three states:
 1. Does nothing - node instructions are emitted.
 2. Calls `assert(c)` if `EMLABCPP_ASSERT_NATIVE` is defined.
 3. Calls `EMLABCPP_ASSERT_FUNC(c)` if `EMLABCPP_ASSERT_FUNC` is defined.

By default, none of the macros are defined.

### bounded.h

Provides `bounded<T,Min,Max>` class that envelops type `T` and provides interface that enforces `T` to remain within bounds `Min` and `Max`. Can be used to relay value constrains in type information.

For example:

```cpp
using power = bounded<int, -1024, 1024>;
```

### concepts.h

A set of C++ concepts designed for implementing a checks inside the library.

### defer.h

Simple utility class to setup code segments executed after the end of scope:

```cpp
{
    defer d = []{
        // do something on exit;
    };
    // ...
}
```
This enforces that "finished" message is send after the `exec_job` call finishes.

### enum.h

Provides a function that converts enum value into string representative, this either does simple `int->string` conversion or uses `magic_enum` library if it is enabled with `EMLABCPP_USE_MAGIC_ENUM` define.

For example:
```cpp
enum e_type {
    FOO = 0,
};

std::cout << convert_enum(FOO) << std::endl;
```

Outputs `FOO` if `magic_enum` is enabled and `0` otherwise.

### iterator.h

Contains `generic_iterator<Derived>` CRTP baseclass.
This simplifies implementation of custom iterators, as most of the methods/operators we expect of iterators can be implemented based on a small set of functions. (operator+, operator++(int), operator++ can be implemetned with operator+=)

For implementing iterator, you only provide the basic subset for this class and it takes care of the rest.
Keep in mind that this is for "general" use case, and for optimallity you may be better served with fully custom iterator.

#### iterators/numeric.h

Iterator that mimics real data container of sequence of numbers. The number is stored inside the iterator and when iterator is advanced, so is the internal value changed. Use functions like `range(from,to)` to creates a range from this iterators.

```cpp
std::vector<int> vec_data = {1,2,3,4,5};

for(std::size_t i : range(vec_data.size()-1))
{
    std::cout << vec_data[i] << '-' << vec_data[i+1] << '\n';
}
```

### match.h

Match is mechanism similar to `std::visit(Callable,Variant)`, but one that changes the order of arguments and allows mutliple callables. The signature is along the lines of: `match(Variant,Callable...)`. The implementation composes callables together and let's method resolution pick the appropaite callable for alternative present in the variant.

This makes it possible to write constructs such as these:
```cpp
std::variant<int, std::string> states;
match(states,
    [&](int x){
        // ...
    },
    [&](std::string&){
        // ...
    });
```
Here, the function executions a lambda for the state that is present in the variant, you can think about it like a `switch` but for variant.

### physical_quantity.h

System of physical quantities based on `quantity.h`.
These represent physical quantity that stores it's unit in it's own type (templated).

This makes it possible to have velocity/length/time represented as distinct types.
Also, the result type of operations like  length divided by time is of type velocity.

This increases safety of physical computations, as it enforces correct units in the math.
The `operator<<` is overloaded to output units for the type, such as: `0.25m`

```cpp
auto uniform_accel = [](distance s0, velocity v0, acceleration a, timeq t) -> distance
{
    return s0 + v0*t + 0.5F*a*t*t;
};

std::cout << distance{0.25};
```

### pid.h

Basic PID regulator implementation using floats, templated based on the time type.

### protocol.h

The protocol library serializes and deserialize C++ data structures into binary messages.
The principle is that the protocol is defined with library types.
Based on the definition, `protocol_handler<Def>` provides routines for serialization and deserialization of data structures corresponding to said definition.

In case of work with simple robot, we can create simple protocol:
```cpp
using distance = unsigned;
using angle = int;

enum robot_cmds : uint8_t
{
    FORWARD = 0,
    LEFT = 1,
    RIGHT = 2
};

struct robot_cmd_group
  : protocol::command_group< std::endian::little >::with_commands<
      protocol::command< FORWARD >::with_args< distance >,
      protocol::command< LEFT >::with_args< angle >,
      protocol::command< RIGHT >::with_args< angle >
    >
{};
```

See examples for more detailed explanation.

### quantity.h

Simple thin overlay over numeric types, that gives abillity to implement strongly typed numeric types.
This is handy in case you want to enforce correctness on type level.
See implementation of `physical_quantity` as an example.


### static_circular_buffer.h

Basic implementation of circular buffer with static maximal size, that can store non-default constructible elements.
No dynamic allocation is used.

```cpp
static_circular_buffer<int, 256> buffr;

for(int i : {0,1,2,3,4,5,6})
{
    buffr.push_back(i);
}

for(int i : buffr)
{
    std::cout << i << ",";
}

while(!buffr.empty())
{
    buffr.pop_front();
}
```

### static_vector.h

Basic implementation of vector with static maximal size, that can store non-default constructible elements.
No dynamic allocation is used.

### types.h

A library of helpers for type inspection, this contains types similar to `type_traits` of standard library.
This follows the pattern of `std::` library - type check is structure with `::value`/`::type` attributes and using for `_v`/`_t` suffixed aliases exists.

```cpp
using data = std::vector<int>;

auto fun = [](int i) -> std::string
{
    return std::to_string(i);
};
using fun_t = decltype(fun);

static_assert(std::is_same_v<mapped_t<data, fun_t>, std::string>);

```

### view.h

Simple container storing a pair of iterators - non-owning container of data.
This make it possible to pass a subset of container to API expecting a container itself.
It is also more sensible for functions that would return `std::pair` of iterators.

```cpp

std::vector<int> vec_data{1,2,3,4,5,6};

for(int i : view{vec_data})
{
    std::cout << i << ',';
}
std::cout << '\n';

for(int i : view{vec_data.begin() + 2, vec_data.end()})
{
    std::cout << i << ',';
}
std::cout << '\n';

for(int i : view_n(vec_data.data(), 4))
{
    std::cout << i << ',';
}
std::cout << '\n';

for(int i : reversed(vec_data))
{
    std::cout << i << ',';
}
std::cout << '\n';
```

### visit.h

`visit` is reimplementation of `std::visit` that has worse time complexity in exchange of less code being generate for the mechanism. It also drop support for multiple variants.

### zip.h

zip iterator over multiple data containers, which dereference value is tuple of references to provided containers.

This is especially handy in combination with numeric iterator. Example is `enumerate(cont)` which returns zip of range over cont size and cont itself, which behaves same as enumerate on python.

```cpp

std::vector<int> vec_data{-1,1,-1,1,-1,1};

for(auto [i,val] : enumerate(vec_data))
{
    std::cout << i << "\t:" << val << '\n';
}

std::vector<std::string> names = {"john", "clark"};
std::vector<std::string> surnames = {"deer", "kent"};

for(auto [name, surname] : zip(names, surnames))
{
    std::cout << name << '\t' << surname << '\n';
}

```
