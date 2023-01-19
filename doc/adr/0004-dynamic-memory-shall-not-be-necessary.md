# 4. Dynamic memory shall not be necessary

Date: 2023-01-19

## Status

Accepted

## Context

The facilities to provide dynamic memory management have non-trivial cost, and the concept of dynamic memory in generall can have bad consequences in embedded system.

Independently on whenever one wants to use dynamic memory, there are use cases/projects in which it is completely forbidden.

## Decision

The library should be implemented in a way that dynamic memory is not necessary, and is not used at all during standard usage.

Any more sophisticated facilities that require some form of memory (for example: because they rely on data container of unknown size) should have allocator support.
These facilities should also optimize design decisions so that they are compatible with simple allocation schemes.

In cases in which allocators are not feasible, the functionality should be hidden behind preprocessor directives by default.

## Consequences

Given the native embedded focus of the library, all the limitations for usage of dynamic memory are to be expected.
(That is, this limitation should not be surprising)

We also assume that as a consequence, the library will contain alternatives to std:: utilities that require dynamic memory, for example: std::function.

The devil is in the details here:
Development of mechanisms that have to use dynamic data structures is still possible and can be done.
It just have to rely on allocator support and giving the user a way to provided static buffers for the containers.
On that topic, it is wise to note that dynamic arrays (std::vector) are a bit tricky for simple memory management schemes, and tree-like containers should be preffered (std::list,std::map).

The preprocessor directive suggestion is for cases like nlohmann::json. JSON third-party library that is quite beneficial for any mechanism that is more directed for desktop usage, as it makes it easy to support conversion into JSON. We do value that library, but it requires dynamic-memory - support for the library is hidden behind preprocessor macro.
