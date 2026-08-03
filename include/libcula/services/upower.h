#pragma once

#ifndef CULA_UPOWER_H
#define CULA_UPOWER_H

#include <stdbool.h>
#include "libcula/utils.h"
#include "libcula/services/dbus.h"

struct cula_context;

// -- Enums & Payloads

/**
 * Types of device UPower properties.
 */
enum cula_upower_device_property_type {
    CULA_UPOWER_DEVICE_PROPERTY_PERCENTAGE,
    CULA_UPOWER_DEVICE_PROPERTY_STATE,
    CULA_UPOWER_DEVICE_PROPERTY_TIME_TO_EMPTY,
    CULA_UPOWER_DEVICE_PROPERTY_TIME_TO_FULL,

    CULA_UPOWER_DEVICE_PROPERTY_NATIVE_PATH,
    CULA_UPOWER_DEVICE_PROPERTY_CAPACITY,
    CULA_UPOWER_DEVICE_PROPERTY_CHARGE_CYCLES,

    CULA_UPOWER_DEVICE_PROPERTY_IS_PRESENT,
    CULA_UPOWER_DEVICE_PROPERTY_POWER_SUPPLY,
};

/**
 * Cula device property change event.
 */
struct cula_upower_device_property_change_evt {
    enum cula_upower_device_property_type type;
    struct cula_upower_device *device;
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
    cula_list_t devices; // cula_upower_device

    struct {
        bool on_battery;
        bool lid_closed;
        bool lid_present;
    } data;

    struct {
        cula_signal_t on_battery_changed; // cula_upower
        cula_signal_t lid_closed_changed; // cula_upower 
        cula_signal_t lid_present_changed; // cula_upower
    } events;

    struct {
        cula_listener_t onchange;
        struct cula_dbus_call_ctx *onchange_cctx;

        cula_listener_t destroy;
    } CULA_INTERNAL;
};

/**
 * A upower device like "BAT0".
 */
struct cula_upower_device {
    struct cula_upower *upower;
    const char *path;

    struct {
        double percentage; 
        uint32_t state;
        int64_t time_to_empty;
        int64_t time_to_full;

        const char *native_path;
        double capacity;
        int32_t charge_cycles; // -1 if unsupported

        bool is_present;
        bool power_supply; // i.e is primary power source?
    } data;

    struct {
        cula_signal_t property_changed; // cula_upower_root_property_change_evt
    } events;
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
