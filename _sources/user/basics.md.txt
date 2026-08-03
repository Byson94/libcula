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

Here is a brief summary of how the event system works. In libcula, a signal is of the type `cula_signal_t`.
To connect to a signal, you need a `cula_listener_t`. You can attach a callback function to the listener
by adding it to the `.notify` field.

```{code-block} c
:linenos:

#include <stdlib.h>

// Your callback function
void listen_handler(cula_listener_t listener, void *data) {
    return;
}

// Attaching it to a listener
cula_listener_t my_listener = calloc(1, sizeof(cula_listener_t));
my_listener.notify = listen_handler;
```

Now that you have a listener, you connect it to a signal with the `cula_signal_add` function:

```{code-block} c
:linenos:
:emphasize-lines: 4

cula_listener_t my_listener = calloc(1, sizeof(cula_listener_t));
my_listener.notify = listen_handler;

cula_signal_add(the_event_signal, my_listener);
```

Don't worry about where `the_event_signal` came from. The signals are usually provided in the service
structure. As its a brief explanation, I **don't** expect you to understand all of it, and its fine.
You'll understand it once you see a more elaborate explanation later with examples.

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
