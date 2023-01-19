# 6. Everything should be in emlabcpp:: namespace

Date: 2023-01-19

## Status

Accepted

## Context

C++ namespaces are a tool to isolate code in a named environment that prevents name collisions in case the code is used in different project.

## Decision

All code of emlabcpp shall be in emlabcpp:: namespace.

## Consequences

It should be quite easy and straightforward to integrate emlabcpp into any project, as the namespace prevents any collisions.
