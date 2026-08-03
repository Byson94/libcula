# Using Services

Let's start off with using a simple service in cula, the UPower service. We will properly cover the
event system, and the common patterns too along with the implementation of this service.

Every service definition headers are provided at `libcula/services/*.h`. So, the upower service header
is at `libcula/services/upower.h`.

## Service Creation

Creating the service is simple. You just have to call the `cula_create_upower(ctx)` function,
which returns a `struct cula_upower`.

On creation itself, it will come with a population `data` field:

```c
struct {
    double percentage;
    bool on_battery;
} data;
```

You can access these to get the current state fetched by the cula UPower service.

## Event System

We have went over a brief of the event system before. But this section will cover all
that is to the event system.

### Attaching to Signal

The `struct cula_upower` has a `events` field which contains all the events supported
by the service:

```c
struct {
    cula_signal_t percentage_changed;
    cula_signal_t on_battery_changed;
} events;
```

You can create your `cula_listener_t` and hook into a signal with `cula_signal_add` like so:

```{code-block} c
:linenos:

// ctx = cula context;
struct cula_upower *upower = cula_create_upower(ctx);

cula_listener_t *listener = calloc(1, sizeof(cula_listener_t));
listener.notify = callback_function;

cula_signal_add(upower->events.percentage_changed, listener);
```

Here we are heap allocating a listener to attach the event to. Instead of that,
we highly recommend you create a dedicated structure where you will store your listeners.
This way, you only have to calloc that structure, and all the listeners will be initialized.
There is also a bonus benifit to it, which I will get to soon after this example:

```{code-block} c
:linenos:

struct upower_wrapper {
    cula_listener_t perc;
    // store any other data you want here
};

// initialize only once
struct upower_wrapper *upower_wrap = calloc(1, sizeof(struct upower_wrapper));

// then use all the listeners freely
upower_wrap->perc.notify = /* .. */;
cula_signal_add(/* .. */, upower_wrap->perc);
```

### Callback Function

We have discussed how we can create the servie and how we can hook into an event.
Now lets discss how you handle things inside the callback function you attached 
to `.notify`.

```{note}
All callbacks must be of the signature `(cula_listener_t *listener, void *data)`.
```

```{code-block} c
:linenos:

void callback_function(cula_listener_t *listener, void *data) {
    struct upower_wrapper *upower_wrap = 
        cula_container_of(listener, upower_wrap, perc);

    // data can be casted into the appropriate type.
}
```

`cula_container_of` is the highlight here. It is inspired by `wl_container_of` from wayland, which
can be used to retreive a pointer to the parent structure from a member name.

```c
// p1: pointer to listener
// p2: pointer to data to create
// p3: member of parent
cula_container_of(p1, p2, p3);
```

```{warning}
The third parameter (member of parent), should be the `cula_listener_t` member to who's
`.notify` you attached the callback to.
```

This is why we recommended you to use a structure instead of calloc'ing a listener
when needed. You can hold other data too like pointer to a global structure
where you store all the data.

## Full Example

Here is the full example with recommended parctices followed.

```{code-block} c
:linenos:

#include <stdio.h>
#include <stdlib.h>
#include <libcula/core.h>
#include <libcula/utils.h>
#include <libcula/services/upower.h>

// Structure for storing upower service events
struct upower_handler {
    cula_listener_t percentage;
    // other service data here...
};

void percentage_notify(cula_listener_t *listener, void *data);

int main() {
    struct cula_context *ctx = cula_create_context();
    if (!ctx) {
        printf("Failed to create context!\n");
        return 1;
    }

    cula_run_context(ctx);

    // Ideally, store this ctx somewhere now.

    /** 
     * Create the service structure.
     * We are heap allocating it with 'calloc' to preserve it beyond
     * the end of this function. Make sure to free it during shutdown.
     */
    struct upower_handler *upower_handler = calloc(1, sizeof(struct upower_handler));

    // Now lets create the service.
    struct cula_upower *upower = cula_create_upower(ctx);
    
    // On creation itself, the upower service will fetch
    // all the initial data. You can access it directly here.
    bool is_on_battery = upower->data.on_battery;
    bool is_lid_closed = upower->data.lid_closed;
    bool is_lid_present = upower->data.lid_present;

    // Now, we can link to the events
    upower_handler->percentage.notify = percentage_notify;
    cula_signal_add(&upower->events.on_battery_changed, &upower_handler->percentage);

    // Shutdown
    cula_list_remove(&upower_handler->percentage.link);
    free(upower_handler);
    cula_destroy_context(ctx);

    return 0;
}

void percentage_notify(cula_listener_t *listener, void *data) {
    struct upower_handler *upower_handler = 
        cula_container_of(listener, upower_handler, percentage);

    // The data can be anything. Make sure to check the API documentation
    // of the event to know which structure it provides, if it provides any.
    // UPower Service passes the 'cula_upower' as data.
    struct cula_upower *upower = data;

    // The upower->data now contains fresh data. Do whatever with it,
    // like storing it in a structure. This is why we recommended using
    // a dedicated structure (like upower_handler) instead of calloc'ing a 
    // new 'cula_listener_t' every time.
}
```
