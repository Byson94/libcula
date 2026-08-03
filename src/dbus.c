#include <stdlib.h>
#include <string.h>
#include <neco.h>
#include <systemd/sd-bus-protocol.h>
#include <uv.h>

#include "libcula/core.h"
#include "libcula/services/dbus.h"
#include "internal/core.h"
#include "internal/dbus.h"
#include "libcula/utils.h"

static void cula_dbus_on_poll(uv_poll_t *handle, int status, int events) {
    if (status < 0) {
        return; 
    }

    struct cula_dbus_poll *dbus_poll = handle->data;
    
    if (events & UV_READABLE) {
        while (sd_bus_process(dbus_poll->dbus->bus, NULL) > 0) {
            // Flush the que
        }
    }
}

static void destroy_dbus_instruction(cula_listener_t *listener, void *data) {
    UNUSED(data);
    struct cula_dbus *dbus = cula_container_of(listener, dbus, DBUS_INTERNAL.destroy);
    cula_destroy_dbus(dbus);
}

struct cula_dbus_poll_setup {
    struct cula_context *ctx;
    struct cula_dbus *dbus;
    int bus_fd;
};

static void cula_dbus_init_poll_task(void *arg) {
    struct cula_dbus_poll_setup *setup = arg;
    
    struct cula_dbus_poll *dbus_poll = calloc(1, sizeof(struct cula_dbus_poll));
    dbus_poll->dbus = setup->dbus;
    
    uv_poll_init(&setup->ctx->loop, &dbus_poll->poll_handle, setup->bus_fd);
    dbus_poll->poll_handle.data = dbus_poll;
    uv_poll_start(&dbus_poll->poll_handle, UV_READABLE, cula_dbus_on_poll);

    setup->dbus->DBUS_INTERNAL.poll = dbus_poll;
    
    free(setup);
}

struct cula_dbus *cula_get_or_create_dbus(struct cula_context *ctx, enum cula_dbus_type bus_type) {
    if (!ctx) {
        return NULL;
    }

    const char *dbus_id = "cula.service.dbus";

    struct cula_service *existing;
    cula_list_for_each(existing, &ctx->services, link) {
        if (strcmp(existing->name, dbus_id) == 0) {
            struct cula_dbus *dbus = existing->service_ptr;
            if (dbus->bus_type == bus_type) return dbus;
        }
    }

    struct cula_dbus *dbus = calloc(1, sizeof(struct cula_dbus));
    dbus->context = ctx;
    dbus->bus_type = bus_type;

    int r = (dbus->bus_type == CULA_DBUS_TYPE_SYSTEM) 
        ? sd_bus_open_system(&dbus->bus) 
        : sd_bus_open_user(&dbus->bus);

    if (r < 0) {
        dbus->status = CULA_DBUS_STATUS_FAILED;
        return NULL;
    }
    dbus->status = CULA_DBUS_STATUS_CONNECTED;

    struct cula_dbus_poll *dbus_poll = calloc(1, sizeof(struct cula_dbus_poll));
    dbus_poll->dbus = dbus;
    
    int bus_fd = sd_bus_get_fd(dbus->bus);
    if (bus_fd >= 0) {
        struct cula_dbus_poll_setup *setup = malloc(sizeof(struct cula_dbus_poll_setup));
        setup->ctx = ctx;
        setup->dbus = dbus;
        setup->bus_fd = bus_fd;

        cula_post_context(ctx, cula_dbus_init_poll_task, setup);
    }

    dbus->DBUS_INTERNAL.poll = dbus_poll;
    cula_list_init(&dbus->DBUS_INTERNAL.calls);

    // Register service into context's service list
    struct cula_service *service = calloc(1, sizeof(struct cula_service));
    service->service_ptr = dbus;
    service->name = dbus_id;
    cula_signal_init(&service->destroy_signal);

    dbus->DBUS_INTERNAL.destroy.notify = destroy_dbus_instruction;
    dbus->service = service;

    cula_list_insert(&ctx->services, &service->link);
    cula_signal_add(&service->destroy_signal, &dbus->DBUS_INTERNAL.destroy);

    return dbus;
}

static int cula_dbus_async_callback(sd_bus_message *m, void *userdata, sd_bus_error *ret_error) {
    struct cula_dbus_call_ctx *call_ctx = userdata;
    UNUSED(ret_error);

    struct cula_dbus_call_result *call_result = calloc(1, sizeof(struct cula_dbus_call_result));

    call_result->reply = m;
    sd_bus_message_ref(m);

    char type_code = 0;
    sd_bus_message_peek_type(m, &type_code, NULL);

    char buf[64];
    switch (type_code) {
        case SD_BUS_TYPE_STRING:       // 's'
        case SD_BUS_TYPE_OBJECT_PATH:  // 'o' 
        case SD_BUS_TYPE_SIGNATURE:    // 'g'
        {
            const char *val = NULL;
            sd_bus_message_read(m, type_code == SD_BUS_TYPE_OBJECT_PATH ? "o" : (type_code == SD_BUS_TYPE_SIGNATURE ? "g" : "s"), &val);
            call_result->str_reply = strdup(val ? val : "");
            break;
        }
        case SD_BUS_TYPE_INT32:        // 'i'
        {
            int32_t val = 0;
            sd_bus_message_read(m, "i", &val);
            snprintf(buf, sizeof(buf), "%d", val);
            call_result->str_reply = strdup(buf);
            break;
        }
        case SD_BUS_TYPE_UINT32:       // 'u'
        {
            uint32_t val = 0;
            sd_bus_message_read(m, "u", &val);
            snprintf(buf, sizeof(buf), "%u", val);
            call_result->str_reply = strdup(buf);
            break;
        }
        case SD_BUS_TYPE_INT64:        // 'x'
        {
            int64_t val = 0;
            sd_bus_message_read(m, "x", &val);
            snprintf(buf, sizeof(buf), "%ld", (long)val);
            call_result->str_reply = strdup(buf);
            break;
        }
        case SD_BUS_TYPE_UINT64:       // 't'
        {
            uint64_t val = 0;
            sd_bus_message_read(m, "t", &val);
            snprintf(buf, sizeof(buf), "%lu", (unsigned long)val);
            call_result->str_reply = strdup(buf);
            break;
        }
        case SD_BUS_TYPE_BOOLEAN:      // 'b'
        {
            int val = 0;
            sd_bus_message_read(m, "b", &val);
            call_result->str_reply = strdup(val ? "true" : "false");
            break;
        }
        case SD_BUS_TYPE_DOUBLE:       // 'd'
        {
            double val = 0.0;
            sd_bus_message_read(m, "d", &val);
            snprintf(buf, sizeof(buf), "%f", val);
            call_result->str_reply = strdup(buf);
            break;
        }
        default:
            call_result->str_reply = strdup("(complex or unhandled type)");
            break;
    }

    cula_list_insert(&call_ctx->results, &call_result->link);
    cula_signal_emit(&call_ctx->events.result, call_result);

    return 0;
}

static void cula_dbus_call_task(int argc, void *argv[]) {
    UNUSED(argc);
    struct cula_dbus_call_ctx *call_ctx = argv[0];
    enum cula_dbus_call_ctx_type call_type = *(enum cula_dbus_call_ctx_type *)argv[1];

    struct cula_dbus *dbus = call_ctx->dbus;

    if (call_type == CULA_DBUS_CALL_TYPE_ONESHOT) {
        if (!call_ctx->message) {
            return;
        }

        sd_bus_error error = SD_BUS_ERROR_NULL;
        sd_bus_slot *slot = NULL;

        int r = sd_bus_call_async(dbus->bus, &slot, call_ctx->message, cula_dbus_async_callback, call_ctx, 0);
        
        call_ctx->message = NULL; 

        if (r < 0) {
            sd_bus_error_free(&error);
            return;
        }
    } else if (call_type == CULA_DBUS_CALL_TYPE_LISTEN) {
        sd_bus_slot *slot = NULL;

        char match_buf[512];
        int offset = snprintf(match_buf, sizeof(match_buf), "type='signal'");
        
        if (call_ctx->destination) {
            offset += snprintf(match_buf + offset, sizeof(match_buf) - offset, ",sender='%s'", call_ctx->destination);
        }
        if (call_ctx->path) {
            offset += snprintf(match_buf + offset, sizeof(match_buf) - offset, ",path='%s'", call_ctx->path);
        }
        if (call_ctx->interface) {
            offset += snprintf(match_buf + offset, sizeof(match_buf) - offset, ",interface='%s'", call_ctx->interface);
        }
        if (call_ctx->method) {
            snprintf(match_buf + offset, sizeof(match_buf) - offset, ",member='%s'", call_ctx->method);
        }

        int r = sd_bus_add_match(
            dbus->bus,
            &slot,
            match_buf,
            cula_dbus_async_callback,
            call_ctx
        );

        if (r < 0) {
            return;
        }

        call_ctx->slot = slot;
    }
}

struct cula_dbus_call_ctx *cula_create_dbus_call(struct cula_dbus *dbus, const char *dest, const char *path, 
                          const char *iface, const char *method, const char *types, ...) {
    struct cula_dbus_call_ctx *call_ctx = calloc(1, sizeof(struct cula_dbus_call_ctx));
    call_ctx->dbus = dbus;
    call_ctx->destination = dest;
    call_ctx->path = path;
    call_ctx->interface = iface;
    call_ctx->method = method;

    cula_list_init(&call_ctx->results);
    cula_signal_init(&call_ctx->events.result);

    sd_bus_message *m = NULL;
    sd_bus_message_new_method_call(dbus->bus, &m, dest, path, iface, method);

    if (types) {
        va_list ap;
        va_start(ap, types);
        sd_bus_message_appendv(m, types, ap);
        va_end(ap);
    }
    call_ctx->message = m; 

    cula_list_insert(&dbus->DBUS_INTERNAL.calls, &call_ctx->link);
    return call_ctx;
}

struct cula_dbus_job_args {
    struct cula_dbus_call_ctx *call_ctx;
    enum cula_dbus_call_ctx_type type;
};

static void cula_dbus_job_callback(void *arg) {
    struct cula_dbus_job_args *args = arg;
    neco_start(cula_dbus_call_task, 2, args->call_ctx,&args->type);
    free(args);
}

void cula_call_dbus(struct cula_context *ctx, struct cula_dbus_call_ctx *call_ctx, enum cula_dbus_call_ctx_type type) {
    struct cula_dbus_job_args *args = malloc(sizeof(struct cula_dbus_job_args));
    if (!args) return;

    args->call_ctx = call_ctx;
    args->type = type;

    cula_post_context(ctx, cula_dbus_job_callback, args);
}

void cula_destroy_dbus_call(struct cula_dbus_call_ctx *call_ctx) {
    if (!call_ctx) return;

    struct cula_dbus_call_result *call_res, *call_tmp;
    cula_list_for_each_safe(call_res, call_tmp, &call_ctx->results, link) {
        cula_list_remove(&call_res->link);
        free(call_res);
    }

    if (call_ctx->slot) {
        sd_bus_slot_unref(call_ctx->slot);
        call_ctx->slot = NULL;
    }
    cula_list_remove(&call_ctx->link);
    free(call_ctx);
}

static void cula_on_poll_closed(uv_handle_t *handle) {
    struct cula_dbus_poll *dbus_poll = handle->data;
    free(dbus_poll->dbus);
    free(dbus_poll);
}

static void cula_stop_poll(void *arg) {
    struct cula_dbus_poll *poll = arg;

    if (!uv_is_closing((uv_handle_t *)&poll->poll_handle)) {
        uv_poll_stop(&poll->poll_handle);
        uv_close((uv_handle_t *)&poll->poll_handle, cula_on_poll_closed);
    }
}

void cula_destroy_dbus(struct cula_dbus *dbus) {
    if (!dbus) {
        return;
    }

    struct cula_dbus_call_ctx *call_ctx, *tmp;
    cula_list_for_each_safe(call_ctx, tmp, &dbus->DBUS_INTERNAL.calls, link) {
        cula_destroy_dbus_call(call_ctx);
    }

    cula_list_remove(&dbus->service->link);

    if (dbus->bus) {
        sd_bus_unref(dbus->bus);
        dbus->bus = NULL;
    }

    if (dbus->DBUS_INTERNAL.poll) {
        struct cula_dbus_poll *poll = dbus->DBUS_INTERNAL.poll;
        dbus->DBUS_INTERNAL.poll = NULL;
        
        cula_post_context(dbus->context, cula_stop_poll, poll);
    } else {
        free(dbus);
    }
}
