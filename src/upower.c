#include <stdlib.h>

#include "libcula/services/upower.h"
#include "libcula/services/dbus.h"
#include "internal/core.h"
#include "libcula/utils.h"

static void on_battery_change(cula_listener_t *listener, void *data) {
    struct cula_upower *upower = cula_container_of(listener, upower, CULA_INTERNAL.on_battery);
    struct cula_dbus_call_ctx *call = data;

    if (call->result != 0) return;

    char type_code = 0;
    sd_bus_message_peek_type(call->reply, &type_code, NULL);

    if (type_code == SD_BUS_TYPE_BOOLEAN) {
        int val = 0;
        sd_bus_message_read(call->reply, "b", &val);
        upower->data.on_battery = val ? true : false;
        cula_signal_emit(&upower->events.on_battery_changed, upower);
    }
}

static void percentage_change(cula_listener_t *listener, void *data) {
    struct cula_upower *upower = cula_container_of(listener, upower, CULA_INTERNAL.percentage);
    struct cula_dbus_call_ctx *call = data;

    if (call->result != 0) return;

    char type_code = 0;
    sd_bus_message_peek_type(call->reply, &type_code, NULL);

    if (type_code == SD_BUS_TYPE_DOUBLE) {
        double val = 0.0;
        sd_bus_message_read(call->reply, "d", &val);
        upower->data.percentage = val;
        cula_signal_emit(&upower->events.on_battery_changed, upower);
    }
}

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
    cula_signal_init(&upower->events.percentage_changed);

    // fetch init data
    struct cula_dbus *dbus = cula_get_or_create_dbus(ctx, CULA_DBUS_TYPE_SYSTEM);
    upower->dbus = dbus;

    // -- First do oneshot
    struct cula_dbus_call_ctx *call_ob = cula_create_dbus_call(dbus, 
            "org.freedesktop.UPower",
            "/org/freedesktop/UPower",
            "org.freedesktop.DBus.Properties",
            "Get",
            "ss", "org.freedesktop.UPower", "OnBattery");

    struct cula_dbus_call_ctx *call_perc = cula_create_dbus_call(dbus, 
            "org.freedesktop.UPower",
            "/org/freedesktop/UPower/devices/display_device",
            "org.freedesktop.DBus.Properties",
            "Get",
            "ss", "org.freedesktop.UPower.Device", "Percentage");

    upower->CULA_INTERNAL.on_battery.notify = on_battery_change;
    cula_signal_add(&call_ob->events.result, &upower->CULA_INTERNAL.on_battery);

    upower->CULA_INTERNAL.percentage.notify = percentage_change;
    cula_signal_add(&call_perc->events.result, &upower->CULA_INTERNAL.percentage);

    cula_call_dbus(ctx, call_ob, CULA_DBUS_CALL_TYPE_ONESHOT);
    cula_call_dbus(ctx, call_perc, CULA_DBUS_CALL_TYPE_ONESHOT);

    // -- Then setup listeners
    struct cula_dbus_call_ctx *listen_ob = cula_create_dbus_call(dbus,
            "org.freedesktop.UPower",
            "/org/freedesktop/UPower",
            "org.freedesktop.DBus.Properties",
            "PropertiesChanged",
            NULL, NULL);

    struct cula_dbus_call_ctx *listen_perc = cula_create_dbus_call(dbus,
            "org.freedesktop.UPower",
            "/org/freedesktop/UPower/devices/display_device",
            "org.freedesktop.DBus.Properties",
            "PropertiesChanged",
            NULL, NULL);

    // Reuse the same listeners
    cula_list_remove(&upower->CULA_INTERNAL.on_battery.link);
    cula_list_remove(&upower->CULA_INTERNAL.percentage.link);
    cula_signal_add(&listen_ob->events.result, &upower->CULA_INTERNAL.on_battery);
    cula_signal_add(&listen_perc->events.result, &upower->CULA_INTERNAL.percentage);

    // Save contexts for cleanup later
    upower->CULA_INTERNAL.onbatt_ctx = listen_ob;
    upower->CULA_INTERNAL.perc_ctx = listen_perc;

    cula_call_dbus(ctx, listen_ob, CULA_DBUS_CALL_TYPE_LISTEN);
    cula_call_dbus(ctx, listen_perc, CULA_DBUS_CALL_TYPE_LISTEN);

    // Setup service
    struct cula_service *service = calloc(1, sizeof(struct cula_service));
    service->service_ptr = dbus;
    service->name = upower_id;
    cula_signal_init(&service->destroy_signal);

    upower->CULA_INTERNAL.destroy.notify = destroy_upower_instruction;
    upower->service = service;

    cula_list_insert(&ctx->services, &service->link);
    cula_signal_add(&service->destroy_signal, &dbus->DBUS_INTERNAL.destroy);

    return upower;
}

void cula_destroy_upower(struct cula_upower *upower) {
    if (upower->CULA_INTERNAL.onbatt_ctx)
        cula_destroy_dbus_call(upower->CULA_INTERNAL.onbatt_ctx);

    if (upower->CULA_INTERNAL.perc_ctx)
        cula_destroy_dbus_call(upower->CULA_INTERNAL.perc_ctx);

    cula_list_remove(&upower->service->link);
    cula_destroy_dbus(upower->dbus);

    free(upower);
}
