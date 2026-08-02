# Basics of libcula

libcula follows an asynchronous, event-driven style of programming. Instead of simple getters which you would have
to poll, libcula provides you with events into which you can hook into and receive noifications whenever the event
goes off. This will save resources as it avoids polling entirely.

## Hello World

Let's now write the "Hello World" of libcula. It does nothing, and just starts a cula context
and destroys it immediately.

```{code-block} c
:linenos:

#include <stdio.h>
#include <libcula/core.h>

int main() {
    struct cula_context *ctx = cula_create_context();
    if (!ctx) {
        printf("Failed to create context!\n");
        return 1;
    }

    cula_run_context(ctx);
    printf("Cula context now running!\n");

    cula_destroy_context(ctx);
    return 0;
}
```

This program exits immediately because it does not do anything with the context, like spawning a service.
Trying to showcase an example of a real service can get out of hand real quickly here as you are just 
starting out. We can look at a few examples of it later.

## Event System

In libcula, a signal is of the type `cula_signal_t`.

## Storing Context

You should always store your cula context in a place where you can access it easily. Like a global variable
or a structure that you pass around everywhere. Instead of storing only the context, we recommend you
create a parent structure that could hold the context, along with the services and their events.

```{code-block} c
:linenos:

struct my_libcula_parent {
    struct cula_context *ctx;
    // services will go here.
};
```

Now, you can store this parent structure directly wherever you are storing it.
