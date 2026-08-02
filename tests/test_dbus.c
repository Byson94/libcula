#include "libcula/utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <libcula/core.h>
#include <libcula/services/dbus.h>

static bool test_completed = false;

static void result_notify(cula_listener *listener, void *data) {
    UNUSED(listener);
    struct cula_dbus_call_ctx *dbus_call = data;
    printf("[%d] Got a result: '%s'\n", dbus_call->result, dbus_call->str_reply ? dbus_call->str_reply : "(null)");
    test_completed = true;
}

int main(void) {
    printf("Running Dbus integration test...\n");

    struct cula_context *ctx = cula_create_context();
    assert(ctx != NULL);

    int res = cula_run_context(ctx);
    assert(res == 0);

    struct cula_dbus *dbus = cula_get_or_create_dbus(ctx, CULA_DBUS_TYPE_SYSTEM);
    if (!dbus) {
        printf("Dbus failed to init!\n");
        cula_destroy_context(ctx);
        return 1;
    }

    struct cula_dbus_call_ctx *dbus_call = cula_create_dbus_call(dbus,
        "org.freedesktop.systemd1",
        "/org/freedesktop/systemd1",
        "org.freedesktop.systemd1.Manager",
        "GetUnit", "s", "ly@tty1.service"
    );

    cula_listener *listener = calloc(1, sizeof(cula_listener));
    listener->notify = result_notify;
    cula_signal_add(&dbus_call->events.result, listener);
    cula_call_dbus(ctx, dbus_call, CULA_DBUS_CALL_TYPE_ONESHOT);

    int timeout = 50;
    while (!test_completed && timeout > 0) {
        usleep(100000);
        timeout--;
    }

    if (!test_completed) {
        printf("Test timed out waiting for D-Bus response!\n");
    }

    cula_destroy_dbus(dbus);
    cula_destroy_context(ctx);
    free(listener);

    printf("Test finished.\n");
    return 0;
}
