#pragma once

#ifndef CULA_UPOWER_H
#define CULA_UPOWER_H

#include <stdbool.h>
#include "libcula/utils.h"
#include "libcula/services/dbus.h"

struct cula_context;

// -- Enums & Payloads

enum cula_performance_degredation_reason {
    CULA_PDR_HIGH_TEMPERATURE,
    CULA_PDR_LAP_DETECTED,
    CULA_PDR_NONE
};

// -- Init / Destroy

struct cula_upower {
    cula_list link;
    struct cula_context *context;
    struct cula_dbus *dbus;

    struct {
        double percentage;
        bool on_battery;
    } data;

    struct {
        cula_signal percentage_changed;
        cula_signal on_battery_changed;
    } events;

    struct {
        cula_listener percentage;
        cula_listener on_battery;
        struct cula_dbus_call_ctx *perc_ctx;
        struct cula_dbus_call_ctx *onbatt_ctx;
    } CULA_INTERNAL;
};

struct cula_upower *cula_create_upower(struct cula_context *ctx);
void cula_destroy_upower(struct cula_upower *upower);

#endif
