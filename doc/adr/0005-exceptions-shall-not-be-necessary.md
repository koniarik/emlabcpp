# 5. Exceptions shall not be necessary

Date: 2023-01-19

## Status

Accepted

## Context

Exceptions are error handling mechanism frequently used in modern C++.
It is however somehow controversial for embedded or for safety critical systems.

There are multiple motivations for avoiding exceptions that might arise:
 - Their error path is considered slow
 - The opinion that they might lead to less stable/robust system, compared to explicit error handling
 - Implementation of exceptions might use dynamic memory

And there are corner case problems, such as:
In case exceptions are used, the system has to have default exception handler for std::terminate which might by called by exceptions. `gcc-arm-none-eabi` provides default handler that brings in C++ type name demangling capability, which results in +-30kbytes of code in the binary. (Which is a LOT for embedded)
The way to avoid is to override the function that gcc uses for this, which is not optimal.

## Decision

Independently of the motivation to avoid or not to avoid exceptions, the decision is that emlabcpp should be designed in a way to work without exceptions as much as possible.
Alternative means of error handling shall be used.

In case system has exceptions enabled, the emlabcpp code should respect that and handle them properly.
(Warning: this might be underdeveloped property of the library)

## Consequences

The error handling becames somehow more noisy, and we still have problems with mechanism such as allocators.
As currently, the std::allocator interface for std:: containers uses only exceptions as a means to provide error information.
(For example: that allocation failed)
