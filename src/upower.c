#include <stdlib.h>

#include "libcula/services/upower.h"
#include "libcula/services/dbus.h"
#include "internal/core.h"
#include "libcula/utils.h"

static void destroy_upower_instruction(cula_listener_t *listener, void *data) {
    UNUSED(data);
    struct cula_upower *upower = cula_container_of(listener, upower, CULA_INTERNAL.destroy);
    cula_destroy_upower(upower);
}

struct cula_upower *cula_get_or_create_upower(struct cula_context *ctx) {
    struct cula_upower *upower = calloc(1, sizeof(struct cula_upower));
    upower->context = ctx;

    // Return if upower already exists
    const char *upower_id = "cula.service.upower";
    struct cula_service *existing;
    cula_list_for_each(existing, &ctx->services, link) {
        if (strcmp(existing->name, upower_id) == 0) {
            struct cula_upower *upower = existing->service_ptr;
            if (upower) return upower;
        }
    }

    // setup signals
    cula_signal_init(&upower->events.on_battery_changed);
    cula_signal_init(&upower->events.lid_present_changed);
    cula_signal_init(&upower->events.lid_closed_changed);

    // fetch init data
    struct cula_dbus *dbus = cula_get_or_create_dbus(ctx, CULA_DBUS_TYPE_SYSTEM);
    upower->dbus = dbus;

    // TODO: Do dbus calls...

    // Setup service
    struct cula_service *service = calloc(1, sizeof(struct cula_service));
    service->service_ptr = upower;
    service->name = upower_id;
    cula_signal_init(&service->destroy_signal);

    upower->CULA_INTERNAL.destroy.notify = destroy_upower_instruction;
    upower->service = service;

    cula_list_insert(&ctx->services, &service->link);
    cula_signal_add(&service->destroy_signal, &upower->CULA_INTERNAL.destroy);

    return upower;
}

void cula_destroy_upower(struct cula_upower *upower) {
    if (upower->CULA_INTERNAL.onchange_cctx)
        cula_destroy_dbus_call(upower->CULA_INTERNAL.onchange_cctx);

    cula_list_remove(&upower->service->link);
    cula_destroy_dbus(upower->dbus);

    free(upower);
}
