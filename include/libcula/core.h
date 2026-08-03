#pragma once

#ifndef CULA_CORE_H
#define CULA_CORE_H

#include <wayland-util.h>

struct cula_context;

/**
 * Create a new cula context. Can be NULL on failure.
 *
 * @return `struct cula_context *` Newly allocated, inactive, cula context.
 */
struct cula_context *cula_create_context(void);

/**
 * Run the provided cula context. 
 *
 * This splits cula into a parallel thread and starts the main loop.
 *
 * @param ctx The inactive cula_context.
 * @return Exit status as int. Non-0 status means error.
 */
int cula_run_context(struct cula_context *ctx);

/**
 * Destory the provided cula context.
 * @param ctx The running cula_context.
 */
void cula_destroy_context(struct cula_context *ctx);

#endif
