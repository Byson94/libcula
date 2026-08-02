#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <uv.h>

#include "libcula/core.h"
#include "internal/core.h"
#include "libcula/utils.h"

static void cula_async_cb(uv_async_t *handle) {
    struct cula_context *ctx = cula_container_of(handle, ctx, async_handle);
    
    cula_list batch;
    cula_list_init(&batch);

    pthread_mutex_lock(&ctx->work_mutex);
    if (!cula_list_empty(&ctx->work_queue)) {
        batch.next = ctx->work_queue.next;
        batch.prev = ctx->work_queue.prev;
        batch.next->prev = &batch;
        batch.prev->next = &batch;
        
        cula_list_init(&ctx->work_queue);
    }
    pthread_mutex_unlock(&ctx->work_mutex);

    struct cula_work_item *item, *tmp;
    cula_list_for_each_safe(item, tmp, &batch, node) {
        cula_list_remove(&item->node);

        if (item->callback) {
            item->callback(item->arg);
        }

        free(item);
    }
}

static void *cula_thread_func(void *arg) {
    struct cula_context *ctx = (struct cula_context *)arg;
    uv_run(&ctx->loop, UV_RUN_DEFAULT);
    
    return NULL;
}

struct cula_context *cula_create_context() {
    struct cula_context *ctx = calloc(1, sizeof(struct cula_context));

    if (uv_loop_init(&ctx->loop) != 0) {
        free(ctx);
        return NULL;
    }

    uv_async_init(&ctx->loop, &ctx->async_handle, cula_async_cb);
    cula_list_init(&ctx->services);
    cula_list_init(&ctx->work_queue);

    return ctx;
}

int cula_run_context(struct cula_context *ctx) {
    ctx->running = true;
    
    int err = pthread_create(&ctx->thread_id, NULL, cula_thread_func, ctx);
    if (err != 0) {
        ctx->running = false;
        return -1;
    }
    
    return 0;
}

void cula_destroy_context(struct cula_context *ctx) {
    uv_stop(&ctx->loop);
    uv_close((uv_handle_t *)&ctx->async_handle, NULL);

    if (ctx->running) {
        pthread_join(ctx->thread_id, NULL);
    }

    uv_loop_close(&ctx->loop);
    free(ctx);
}

void cula_post_context(struct cula_context *ctx, cula_work_cb cb, void *arg) {
    struct cula_work_item *item = malloc(sizeof(struct cula_work_item));
    if (!item) return;

    item->callback = cb;
    item->arg = arg;

    pthread_mutex_lock(&ctx->work_mutex);
    cula_list_insert(&ctx->work_queue, &item->node);
    pthread_mutex_unlock(&ctx->work_mutex);

    uv_async_send(&ctx->async_handle);
}
