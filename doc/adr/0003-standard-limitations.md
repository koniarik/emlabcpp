# 3. Standard limitations

Date: 2023-01-19

## Status

Accepted

## Context

The library has to be compatible for embedded world, that ecosystem is usually limited in capabilities, which have to be taken into account during development.

## Decision

There should be a concept of "standard usage" which greatly limits what can be used in the library and how.
These limits should be well defined, and explained.

Library still can use techniques outside of the defined limits, but all that usage should not affect or limit standard usage.
For the extra facilities, the library can use of two approaches to avoid affecting standard usage:
 - Make the concrete functionality conditional with usage of preprocessor defines (for example: EMLABCPP_USE_DEMANGLING define allows usage of demangling facility of the compiler)
 - Clearly separate the extra functionality into separate file (library provides custom allocator/memory_resource concept, one of the memory resources uses new/delete and is in separate file)

In case preprocessor defines are used:
 - The functionality is disabled by default
 - The preprocessor define enables the functionality if defined, disabled otherwise
 - The define shall be prefixed with EMLABCPP_USE_

The exact limitations should be defined in other ADR records.

## Consequences

The benefit of this decision is that the "standard usage" of the library should be compatible with wide spectrum of embedded environments/setup.

In case it is wanted by the user, there is still abillity to provide/use the advanced capabilities, user just has to explicitly enable it.

Prime example of capability that benefits of this are logging statements used by the library itself.
By default, no logging is happening in any manner and the logging macros leave nothing in the code.
If enabled for host environment, the logging macros will suddenly output the logging statements on stdout. This is extremely beneficial for unit tests of the library and usage of the library in host environment.
As the logging statements makes debugging easier.
