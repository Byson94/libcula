#pragma once

#ifndef CULA_UPOWER_H
#define CULA_UPOWER_H

#include <stdbool.h>
#include "libcula/utils.h"
#include "libcula/services/dbus.h"

struct cula_context;

// -- Enums & Payloads

/**
 * Reason for performance degredation.
 */
enum cula_performance_degredation_reason {
    CULA_PDR_HIGH_TEMPERATURE,
    CULA_PDR_LAP_DETECTED,
    CULA_PDR_NONE
};

// -- Init / Destroy

/**
 * Cula UPower service built on DBus.
 *
 * The UPower Daemon must be active and running for the service
 * to work properly.
 */
struct cula_upower {
    struct cula_service *service;
    struct cula_context *context;
    struct cula_dbus *dbus;

    struct {
        double percentage;
        bool on_battery;
    } data;

    struct {
        cula_signal_t percentage_changed;
        cula_signal_t on_battery_changed;
    } events;

    struct {
        cula_listener_t percentage;
        cula_listener_t on_battery;
        struct cula_dbus_call_ctx *perc_ctx;
        struct cula_dbus_call_ctx *onbatt_ctx;
        cula_listener_t destroy;
    } CULA_INTERNAL;
};

/**
 * Create a new upower service or get an already running one.
 *
 * @param ctx The running cula context.
 * @return `struct cula_upower *` The cula UPower service.
 */
struct cula_upower *cula_get_or_create_upower(struct cula_context *ctx);

/**
 * Destroy the provided cula UPower service.
 *
 * @param upower The upower service.
 */
void cula_destroy_upower(struct cula_upower *upower);

#endif
