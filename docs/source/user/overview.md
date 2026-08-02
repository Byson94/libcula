# Overview

libcula is a highly performant and concurrent C library which abstracts most of the Linux interfaces like
UPower, Networking, DBus, and so on in a comprehensible architecture inspired by wayland.

## Context

The libcula context is an important structure that holds critical information such as the event loop,
services, thread ID, etc. A context must be created to use the services provided by libcula.

This context runs in parallel to your host program, which makes everything ran in libcula non-blocking.
This is further split into green threads, where each service (like dbus) is handled by its own 
coroutine.

## Services

Services are the libcula abstractions of linux interfaces that run on top of the context. There 
are many services in libcula, but they all follow the same event-driven pattern which makes
it easy to pick up. All the service definition headers can be found at `libcula/services/*.h`.

## Event System

Everything in libcula is async and happens on a parallel thread. Instead of simple getters which you have
to poll, you will get highly performant events into which you can hook functions into. These functions
will be called every time the event triggers and you can do the appropriate action there.
