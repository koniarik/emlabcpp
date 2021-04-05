
# emlabcpp

This library is a collection of small tools used by our laboratory in C++ code.
The tools are primary designed for embedded environment - we avoid dynamic allocation of memory.

Repository is at https://gitlab.fi.muni.cz/xkoniar/emlabcpp and mirrored to github repository https://github.com/emlab-fi/emlabcpp 

## algorithm.h
Contains a subset of algorithms from \<algorith\> standard library, with a two major changes:
 1. All functions expects container with begin/end iterators as an input, not the iterators themselves
 2. Functions are able to iterate over std::tuple
 
This is expanded with other short functions representing simple algorithms and necessary stuff.

## either.h

either\<A,B\> is std::variant alternative able to hold only two types.
Either however contains multiple methods to transform it's state and type.
This makes it possible to chain the changes in the code.

## iterator.h

contains generic_iterator\<Derived\> CRTP baseclass. 
This simplifies implementation of custom iterators, as most of the methods/operators we expect of iterators can be implemented based on a small set of functions. (Operator+, Operator++(int), Operator++ can be implemetned with Operator+=)

For implementing iterator, you only provide the basic subset for this class and it takes care of the rest.
Keep in mind that this is for "general" use cas, and for optimal performance you may be better served with fully custom iterator.

### iterators/access.h

Iterators overlays the data stored in input container, and provides access only to 'reference' provided by function out of item in the container.

### iterators/numeric.h

Iterator that mimics real data container of sequence of numbers. The number is stored inside the iterator and when iterator is advanced, so is the internal value changed.

Contains also functions like 'range(from,to)' that creates a range from this iterators.
This makes it possible to write: `for( int i : range(0,5) )`

### iterators/subscript.h

Iterator over datatype that implemented operator[] but does not have iterators

## physical_quantity.h

System of physical quantities based on quantity.hs
These represent physical quantity that remembers it's unit in it's own type (templated).

This makes it possible to have velocity/length/time represented with this template, where each quantity is different type.
Also, the result type of operations like  length divided by time is of type velocity.

This increases safety of physical computations, as it enforces correct units in the math.

## pid.h

UNTESTED

## quantity.h

Simple thin overlay over numeric types, that gives abillity to implement strongly typed numeric types.
This is handy in case you want to enforce correctness on type level

## static_circular_buffer.h

Basic implementation of circular buffer with static maximal size, that can store non-default constructible elements.
No dynamic allocation is used.

## static_vector.h

Basic implementation of vector with static maximal size, that can store non-default constructible elements.
No dynamic allocation is used.

## types.h

A library of handy helpers for type inspaction, mostly using SFINAE, this contains types similar to type_traits of standard library.
This follows the pattern of std:: library - type check is structure with ::value/::type attributes and using for \_v/\_t suffixed aliases exists.

## view.h

Simple container storing a pair of iterators to different container - non-owning container of data.
This is heavily used by other parts of the code, and si similar to std::span introduced in C++20

This makes it possible to use API that expects data container as input, rather then pair of iterators.

## zip.h

zip iterator over multiple data containers, which dereference value is tuple of references to provided containers.

This is especially handy in combination with numeric iterator. Example is `enumerate(cont)` which returns zip of range over cont size and cont itself, which behaves same as enumerate on python.

`for( auto [i, item] : enumerate(cont))`
