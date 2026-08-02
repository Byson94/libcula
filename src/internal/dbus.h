#pragma once

#ifndef CORE_DBUS_INTERNAL_H
#define CORE_DBUS_INTERNAL_H

#include <uv.h>

struct cula_dbus_poll {
    uv_poll_t poll_handle;
    struct cula_dbus *dbus;
};

#endif
