#pragma once

#ifndef CULA_INTERNAL_CORE_H
#define CULA_INTERNAL_CORE_H

#include <uv.h>
#include <pthread.h>
#include <stdbool.h>
#include <wayland-util.h>

#include "libcula/utils.h"

struct cula_service {
    cula_list link;
    const char *name;
    void *service_ptr;
};

typedef void (*cula_work_cb)(void *arg);

struct cula_work_item {
    cula_work_cb callback;
    void *arg;
    cula_list node;
};

struct cula_context {
    uv_loop_t loop;
    pthread_t thread_id;
    bool running;
    cula_list services;
    uv_async_t async_handle;

    pthread_mutex_t work_mutex;
    cula_list work_queue;
};

void cula_post_context(struct cula_context *ctx, cula_work_cb cb, void *arg);

#endif
