#pragma once

#ifndef CULA_DBUS_H
#define CULA_DBUS_H

#include <systemd/sd-bus.h>
#include "libcula/utils.h"

struct cula_context;

/**
 * @breif Type of the dbus interface.
 */
enum cula_dbus_type {
    CULA_DBUS_TYPE_SYSTEM,
    CULA_DBUS_TYPE_SESSION,
};

/*
 * @breif Current state of the dbus interface.
 */
enum cula_dbus_status {
    CULA_DBUS_STATUS_CONNECTED,
    CULA_DBUS_STATUS_FAILED,
};

/**
 * @brief Represents a D-Bus connection service instance.
 */
struct cula_dbus {
    struct cula_service *service;
    struct cula_context *context;
    enum cula_dbus_type bus_type;
    enum cula_dbus_status status;
    sd_bus *bus;
};

/**
 * @brief The call to make to the dbus.
 */
struct cula_dbus_call_ctx {
    struct cula_dbus *dbus;
    const char *destination;
    const char *path;
    const char *interface;
    const char *method;
    sd_bus_message *message;

    sd_bus_slot *slot;
    sd_bus_message *reply;
    const char *str_reply;
    int result;

    struct {
        cula_signal result;
    } events;
};

/*
 * @breif The type of the dbus call.
 */
enum cula_dbus_call_ctx_type {
    CULA_DBUS_CALL_TYPE_ONESHOT,
    CULA_DBUS_CALL_TYPE_LISTEN,
};

/**
 * @brief Create a new D-Bus service instance attached to a context.
 * 
 * @param ctx The cula context running the event loop.
 * @param bus_type The type of bus ("system" or "session").
 * @return struct cula_dbus* Newly allocated D-Bus service or NULL on failure.
 */
struct cula_dbus *cula_get_or_create_dbus(struct cula_context *ctx, enum cula_dbus_type bus_type);

/**
 * @brief Construct a dbus call context.
 *
 * @param dbus The cula dbus to make the call to.
 * @param dest Destination of call (e.g. "org.freedesktop.UPower").
 * @param path Path of call (e.g. "/org/freedesktop/UPower/devices/battery_BAT0").
 * @param iface Interface of call (e.g. "org.freedesktop.UPower.Device").
 * @param method Method of the call (e.g. "Refresh").
 * *param types Types.
 */
struct cula_dbus_call_ctx *cula_create_dbus_call(struct cula_dbus *dbus, const char *dest, const char *path, 
                          const char *iface, const char *method, const char *types, ...);

/*
 * @breif Make a call to the dbus with the specified call context.
 *
 * @param call_ctx The call context with call info.
 * @param type The type of call to make.
 */
void cula_call_dbus(struct cula_context *ctx, struct cula_dbus_call_ctx *call_ctx, enum cula_dbus_call_ctx_type type);

/*
 * @breif Cancle a dbus listen. No-op if not listening.
 */
void cula_cancel_dbus_listen(struct cula_dbus_call_ctx *call_ctx);

/**
 * @brief Destroy a D-Bus service instance and clean up resources.
 */
void cula_destroy_dbus(struct cula_dbus *dbus);

#endif
